/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/tracepp/tracepp.C $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/*
# *** tracepp - Linux trace pre processor
# this one replaces the trace strings by the corresponding hash value

# *** Usage
#
# prepend compiler call with the call of this pre processor, i.e if you have
#      $(CC) $(CFLAGS) -o $@ $<
# in your Makefile change it to this:
#      tracepp $(CC) $(CFLAGS) -o $@ $<
# tracepp will use "$(CC) -E" to call the C pre processor "cpp".
# you can set a env var "REALCPP" to the name of a program to select
# a different programm as cpp
#
# tracepp creates a file "$target.hash" with the trace strings and the hash values.
#
# to enable debug mode set envvar TRACEPPDEBUG to 1 or give '-d' as first arg
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <unistd.h>

#include <sys/types.h>
typedef u_int32_t u32 ;
typedef u_int8_t u8 ;
#include <jhash.h>

using namespace std;

static string version = "2.0";
static string macro_version = "1";

static bool debug = false;
#define dprintf(format, ...) if (debug) { printf(format, ##__VA_ARGS__); fflush(stdout); }
static map<string,string> hashtab;
static string hashtype;

static string tmp;
static string cmd;
static FILE* CPP = NULL; // pipe from preprocessor
static FILE* CC = NULL;  // pipe to compiler
static FILE* DEBUG = NULL;
static FILE* DEBUGIN = NULL;

//*****************************************************************************
// replace_substr
//*****************************************************************************
void replace_substr(std::string& str, const std::string& oldStr, const std::string& newStr)
{
    size_t pos = 0;
    while((pos = str.find(oldStr, pos)) != std::string::npos)
    {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }

}

//*****************************************************************************
// fileparse
//*****************************************************************************
void fileparse(const string& in_str, string& name, string& dir, string& suff)
{
    string str(in_str);
    size_t pos;
    name = "";
    dir = "";
    suff = "";
    pos = str.find_last_of('.');
    if (pos != string::npos)
    {
        suff = str.substr(pos);
        str = str.substr(0, pos);
    }
    pos = str.find_last_of('/');
    if (pos != string::npos)
    {
        name = str.substr(pos+1);
        str = str.substr(0, pos+1);
    }
    dir = str;
}

static const size_t TRACE_ADAL_HASH_LEN = 14;
//*****************************************************************************
// chop_up_line
//*****************************************************************************
bool chop_up_line(string& in_line, string& prefix, string& strings, string& salt, string& suffix)
{
    // First see if this line matches the pattern we're looking for
    // Since this will return false 95%+ of the time this function it called, we do it
    // before doing any other init for performance reasons.
    size_t pos = in_line.find("trace_adal_hash");
    if (pos == string::npos) { return(false); }

    // trace_adal_hash ( "..." ".." "..." , 2 )
    // regex: PREFIX 'trace_adal_hash' space '(' space STRINGS  space ',' space NUMBER space ')' SUFFIX
    // STRINGS:  '"' .* '"' space? +

    // Original perl magic incantation:
    //   while($line =~ m/^(.*?)trace_adal_hash\s*\(\s*((".*?(?<!\\)"\s*)+),\s*(-?\d+)\s*\)(.*)$/) {
    //      ($prefix, $strings, $salt, $suffix) = ($1, $2, $4, $5);
    //
    // Decrypting the magic pattern matching...
    // (.*?)            => $1 = everything up to the word "trace_adal_hash"
    // trace_adal_hash  = delimiter
    // \s*\(\s*         = delimiter = <0-n whitespace chars>, left paren, <0-n whitespace chars>
    // ((".*?(?<!\\)"\s*)+) => $2 = double-quote, some chars up to last closing double-quote ($3 used for nested regex)
    // ,\s*             = delimiter = comma followed by some whitespace
    // (-?\d+)\s*\)(.*) => $4 and $5
    // $/)             = end of the line input string
    string line(in_line);
    prefix = "";
    strings = "";
    salt = "";
    suffix = "";
    size_t pos1;
    size_t pos2;
    size_t pos3;

    pos1 = pos + 15; // pos1 = after "trace_adal_hash"
    pos2 = line.find("(", pos1);
    if (pos2 == string::npos) { return(false); }
    ++pos2;
    pos3 = line.find("\"", pos2);
    if (pos3 == string::npos) { return(false); }
    dprintf("--------\nchop_up_line: Passed basic checks. line= %s\n", line.c_str());
    dprintf("pos1=%d, pos2=%d, pos3=%d\n", pos1, pos2, pos3);
    if ((pos1 != (pos2-1)) && (line.find_first_not_of(" \t", pos1, (pos2-pos1)+1) != string::npos)) { return(false); } //non-whitespace?
    if ((pos2 != pos3) && (line.find_first_not_of(" \t", pos2, (pos3-pos2)) != string::npos)) { return(false); } //non-whitespace?

    // Get the prefix data
    dprintf(">chop_up_line(\"%s\",...)\n", line.c_str());
    prefix = line.substr(0, pos);
    line = line.substr(pos + TRACE_ADAL_HASH_LEN);
    dprintf("    prefix=\"%s\"\n", prefix.c_str());

    // Get the strings and join/fix them: Store all strings between paired double-quotes up to the
    // first comma not inside a string
    pos = line.find_first_of('(');
    if (pos == string::npos) { return(false); }
    line = line.substr(pos + 1);
    strings = "";
    while(!line.empty())
    {
        pos = line.find_first_of(',');
        pos1 = line.find_first_of('"');
        if ((pos1 == string::npos) || ((pos != string::npos) && (pos < pos1))) { break; } // found comma before next quote
        pos2 = line.find_first_of('"', (pos1+1));
        if (pos2 == string::npos) { return(false); } // unbalanced quotes!
        while(line[pos2-1] == '\\') // skip escaped quotes in the string (they're not the ending quote)
        {
            pos2 = line.find_first_of('"', (pos2+1));
            if (pos2 == string::npos) { return(false); } // unbalanced quotes!
        }
        if (!strings.empty()) { strings += " "; }
        strings += line.substr(pos1, (pos2-pos1)+1);
        line = line.substr(pos2+1);
    }
    replace_substr(strings, "\" \"", "");
    replace_substr(strings, "\\\"", "ESCAPEDQUOTE");
    replace_substr(strings, "\"", "");
    replace_substr(strings, "ESCAPEDQUOTE", "\"");
    // Remove trailing whitespace
    pos = strings.find_last_not_of(" \t\n");
    if ((pos != string::npos) && (pos < (strings.length()-1)))
    {
        strings = strings.substr(0, pos+1);
    }

    dprintf("    strings>%s<\n", strings.c_str());

    // Get the salt
    pos = line.find(",");
    if (pos != string::npos) { line = line.substr(pos+1); }
    pos = line.find_first_of(')');
    if (pos == string::npos) { return(false); }
    salt = line.substr(0, pos);
    line = line.substr(pos+1);
    //dprintf("    salt=\"%s\"\n", salt.c_str());
    pos = salt.find_first_not_of(" \t\n");
    if (pos == string::npos) { return(false); }
    salt = salt.substr(pos);
    pos = salt.find_last_not_of(" \t\n");
    if (pos == string::npos) { return(false); }
    salt = salt.substr(0, pos+1);
    dprintf("    salt=\"%s\"\n", salt.c_str());

    // Get the suffix (i.e. the rest)
    suffix = line;
    if (suffix[suffix.length()-1] == '\n') { suffix = suffix.substr(0, suffix.length()-1); }
    dprintf("    suffix=\"%s\"\n<chop_up_line() returning true\n", suffix.c_str());

    return(true);
}

//*****************************************************************************
// get_format_string
//*****************************************************************************
int get_format_string(const string& in_str, string& format)
{
    int format_salt = 0;
    size_t pos;
    size_t pos_end;
    string str(in_str);
    // (@format_param) = ($strings =~ /(%[#0\- +'I]*\d*(?:\.\d*)?[hlLqjzt]*[diouxXeEfFgGaAcsCSpn])/g);
    // $format = join(',', @format_param);
    // Decrypting the regular expression magic...
    // (%[#0\- +'I]*\d*(?:\.\d*)?[hlLqjzt]*[diouxXeEfFgGaAcsCSpn])

    format = "";
    while(!str.empty())
    {
        pos = str.find("%");
        if (pos == string::npos) { break; }
        if (pos == (str.length()-1)) { break; } // last char in string?  just skip it
        if (str[pos+1] == '%') // double percent sign?  just skip first one
        {
            str = str.substr(pos+1);
            continue;
        }
        pos_end = str.find_first_of("cdieEfgGosuxXpn", pos); // find formatting specifier
        if (pos_end == string::npos)
        {
            fprintf(stderr, "ERROR: tracepp could not parse trace formatting string \"%s\" in \"%s\"\n", str.c_str(), in_str.c_str());
            break;
        }

        if (!format.empty())
            format += ",";
        format += str.substr(pos, (pos_end-pos)+1);
        ++format_salt;

        str = str.substr(pos_end+1);
    }
    // Correct for escaped percent signs
    string temp_str(in_str);
    while((pos = temp_str.find("%%")) != string::npos)
    {
        if (pos < (temp_str.length()-2)) // Not the last thing in the string?
        {
            dprintf("    decrementing salt value %d\n", format_salt);
            --format_salt;
            temp_str = temp_str.substr(pos+2);
        }
        else
        {
            temp_str = "";
        }
    }

    return(format_salt);
}

#define BUF_SIZE 40
//*****************************************************************************
// get_hash
//*****************************************************************************
void get_hash(const string& i_str, const unsigned int salt_num, string& hash)
{
    string str(i_str);
    unsigned int hash_num = 0;
    char buf[BUF_SIZE];

    hash = "";
    dprintf(">get_hash(\"%s\",%d)\n", str.c_str(), salt_num);

    // Call jhash function to get the hash value
    hash_num = jhash((void*)str.c_str(), str.length(), salt_num);
    dprintf("jhash() returned: %u\n", hash_num);
    sprintf(buf, "%u", hash_num);
    hash = buf;

    // validate the hash value
    size_t pos = hash.find_first_not_of("0123456789");
    if (pos != string::npos)
    {
        fprintf(stderr, "trexhash error: %s\n", hash.c_str());
        fprintf(stderr, "for call <<%s>>\n", cmd.c_str());
        exit(1);
    }

    // If hash is empty, use the sum of the ord values in the original string
    if ((hash == "")||(hash == "0"))
    {
        unsigned int len = str.length();
        unsigned int hash_num = 0;
        //unsigned char conv_buf[2] = { '\0', '\0' };
        u_int8_t conv_num;
        for (unsigned int i=0; i < len; ++i)
        {
            //conv_buf[0] = str[i];
            conv_num = (u_int8_t)str[i];
            hash_num += (unsigned int)conv_num;
        }
    }
    dprintf("<get_hash(\"%s\",%d) returned hash: %s\n", str.c_str(), salt_num, hash.c_str());
}

//*****************************************************************************
// parse_line
//*****************************************************************************
void parse_line(map<string,string>& rhash, string& line, string& out_line)
{
    // NOTE: "line" arg may get modified by this function!  Caller must not assume it's unchanged.
    string format;
    string prefix;
    string strings;
    string tmp;
    string salt;
    string hash;
    int salt_num;
    int format_salt;
    string suffix;
    string write_all_suffix;
    size_t pos;

    out_line = "";
    // trace_adal_hash ( "..." ".." "..." , 2 )
    // regex: PREFIX 'trace_adal_hash' space '(' space STRINGS  space ',' space NUMBER space ')' SUFFIX
    // STRINGS:  '"' .* '"' space? +
    //while($line =~ m/^(.*?)trace_adal_hash\s*\(\s*((".*?(?<!\\)"\s*)+),\s*(-?\d+)\s*\)(.*)$/) {
    // Attempt to approximate the above amazing perl regex...
    while( chop_up_line(line, prefix, strings, salt, suffix) )
    {
        //dprintf("\n\nprefix = %s\nstrings = %s\nsalt = %s\nsuffix = %s\n",
        // prefix.c_str(), strings.c_str(), salt.c_str(), suffix.c_str());
        // is this a trace_adal_write_all call?
        pos = prefix.find("trace_adal_write_all");
        if (pos != string::npos)
        {
            // yes. replace trace_adal_hash with hash value and reduced format string
            format_salt = get_format_string(strings, format);
            // reduced format string will be added after hash value
            write_all_suffix = ",\" ";
            write_all_suffix += format;
            write_all_suffix += "\"";

            if (!salt.empty())
            {
                salt_num = atoi(salt.c_str());
            }
            else
            {
                salt_num = -1;
            }

            if (salt_num == -1)
            {
                salt_num = format_salt;
            }
            else if (salt_num != format_salt)
            {
                fprintf(stderr, "ERROR: printf mismatch in '%s': TRACE says %d, format says %d args\n",
                        line.c_str(), salt_num, format_salt);
            }
        }
        else
        {
            write_all_suffix = "";
            salt_num = atoi(salt.c_str());
        }

        // get the trex hash value for the strings
        get_hash(strings, salt_num, hash);

        // check for duplicate and store the mapping
        if ((rhash.find(hash) != rhash.end()) && (rhash[hash] != strings))
        {
            fprintf(stderr, "hash collision: two different strings give the same hash value '%s'\n", hash.c_str());
            fprintf(stderr, "%s\n%s\n", strings.c_str(), rhash[hash].c_str());
            return;
        }
        rhash[hash] = strings;

        // add the modified line to the output
        tmp = prefix;
        tmp += "((";
        tmp += hashtype;
        tmp += ") ";
        tmp += hash;
        tmp += "U)";
        tmp += write_all_suffix;
        out_line += tmp;
        dprintf("changed call: %s...\n", tmp.c_str());
        line = suffix;
    }
    out_line += line;
    if (out_line[out_line.length()-1] != '\n')
        out_line += "\n";
}

//*****************************************************************************
//                                 main
//*****************************************************************************
int main(int argc, char** argv)
{
    char* p_env = getenv("TRACEPPDEBUG");
    if (p_env)
        debug = true;


    int argi = 1;
    string arg;
    string optarg;
    if (argc > 1) arg = argv[1];
    if ((argc < 2) || (arg == "-h"))
    {
        fprintf(stderr, "usage: %s realcompiler compileroptions -o target source\n", argv[0]);
        exit(9);
    }
    string realcc(argv[argi++]);
    string cctype("c++");
    bool optx_found = false;

    if (realcc == "-d")
    {
        debug = true;
        realcc = argv[argi++];
    }

    // wait until -d options is handled before checking $debug
    dprintf("tracepp version %s - API/macro version %s\n", version.c_str(), macro_version.c_str());

    p_env = getenv("REALCPP");
    string realcpp;
    if (p_env)
        realcpp = p_env;
    if (realcpp.empty())
    {
        dprintf("cannot find cpp, using <realcompiler> -E\n");
        realcpp = realcc;
        realcpp += " -E";
    }
    dprintf("realcpp is %s\n", realcpp.c_str());

    //------------------------------------------------------------------------------
    // parse all the arguments
    //------------------------------------------------------------------------------
    string source;
    string object;
    vector<string> ccopts;
    vector<string> cppopts;
    bool dodeps = false;
    string depfile;
    string pfx;
    string sfx;
    int origargi = argi;
    for( ; argi < argc; ++argi)
    {
        arg = argv[argi];
        dprintf("Processing argv[%d]: \"%s\"\n", argi, arg.c_str());
        if (arg.length() > 2)
        {
            pfx = arg.substr(0,2);
            sfx = arg.substr(arg.length()-2);
        }
        else
        {
            pfx = arg;
            sfx = arg;
        }
        dprintf("   pfx: \"%s\"    sfx: \"%s\"\n", pfx.c_str(), sfx.c_str());

        if (pfx == "-o")
        {
            if (! object.empty())
            {
                fprintf(stderr, "two -o options, aborting\n");
                exit(1);
            }
            if (arg.length() > 2)
            {
                object = sfx;
            }
            else
            {
                object = argv[++argi];
            }
            dprintf("object is now %s\n", object.c_str());
        }
        else if (arg == "-c")
        {
            // don't call cpp with -c, this is for the compiler
            ccopts.push_back(arg);
            dprintf("found -c option\n");
        }
        else if (pfx == "-l")
        {
            // cpp doesn't need library arguments
            cppopts.push_back(arg);
        }
        else if (pfx == "-i")
        {
            // option takes an argument, handle it too
            optarg = argv[argi++];
            ccopts.push_back(arg);
            ccopts.push_back(optarg);
            cppopts.push_back(arg);
            cppopts.push_back(optarg);
            dprintf("found option '%s %s'\n", arg.c_str(), optarg.c_str());
        }
        else if ((arg == "-L") ||
                 (arg == "-I") ||
                 (arg == "-x") ||
                 (arg == "-b") ||
                 (arg == "-B") ||
                 (arg == "-V") ||
                 (arg == "-D") ||
                 (arg == "--param") ||
                 (arg == "-MQ") ||
                 (arg == "-MT"))
        {
            // option takes an argument, handle it too
            optarg = argv[argi++];
            ccopts.push_back(arg);
            ccopts.push_back(optarg);
            cppopts.push_back(arg);
            cppopts.push_back(optarg);
            dprintf("found option '%s %s'\n", arg.c_str(), optarg.c_str());
            if (arg == "-x")
            {
                // option x sets the language - c or c++
                if ((optarg != "c") && (optarg != "c++"))
                {
                    fprintf(stderr, "cannot process language '%s', aborting\n", optarg.c_str());
                    exit(1);
                }
                cctype = optarg;
                optx_found = true;
            }
        }
        else if ((arg == "-MD")||(arg == "-MG"))
        {
            // gen deps
            dodeps = true;
            dprintf("found %s, creating dependencies\n", arg.c_str());
        }
        else if (arg == "-MF")
        {
            // set dependencies file
            depfile = argv[argi++];
            dprintf("set dependencies file to '%s'\n", depfile.c_str());
        }
        else if (arg[0] == '-')
        {
            // arg starts with - so it's an option
            ccopts.push_back(arg);
            cppopts.push_back(arg);
            dprintf("found option '%s'\n", arg.c_str());
        }
        else if ((sfx == ".a") ||
                 (sfx == ".o"))
        {
            // an object or archive, ignore this but give it to cc
            ccopts.push_back(arg);
            dprintf("found object/archive '%s'\n", arg.c_str());
        }
        else if ((sfx == ".c") ||
                 (sfx == ".C") ||
                 (arg.substr(arg.length()-4) == ".cpp") ||
                 (arg.substr(arg.length()-4) == ".cxx"))
        {
            // the source file(s). we should only get one
            if (!source.empty())
            {
                fprintf(stderr, "don't know to handle two source files, aborting\n");
                exit(1);
            }
            source = arg;
            // put the - (for read-from-stdin) where the source file was
            // (order is important!)
            ccopts.push_back("-");
            dprintf("found source file %s\n", source.c_str());
        }
        else if (access(arg.c_str(), F_OK))
        {
            // option but not a file, an unknown option?
            ccopts.push_back(arg);
            cppopts.push_back(arg);
            dprintf("found unknown option '%s'\n", arg.c_str());
        }
    }

    //------------------------------------------------------------------------------
    // set other parameters based on arguments specified
    //------------------------------------------------------------------------------
    if (source.empty())
    {
        // this might be a call to link a program instead of compile a source (or asm source)
        dprintf("NOTME: starting as cc '%s ...'\n", realcc.c_str());
        execvp(realcc.c_str(), &(argv[origargi]));
        fprintf(stderr, "ERROR: returned from execvp() call to run %s\n", realcc.c_str());
    }
    if (object.empty())
    {
        dprintf("no object file given, default to source name\n");
        // gcc builds object name from source name if no -o given, replacing
        // suffix with .o. The file is placed in the current directory,
        // not in the source directory!
        string n;
        string d;
        string s;
        fileparse(source, n, d, s);
        if (!n.empty() && !s.empty())
        {
            object = n + ".o";
            dprintf("tracpp: guessing object name %s\n", object.c_str());
            dprintf("        from source name     %s\n", source.c_str());
        }
        else
        {
            fprintf(stderr, "Unable to determine Source File Name\n");
            exit(1);;
        }
    }

    // set value of trace hash according to language
    // check source file extension if no explicit -x option given
    if (!optx_found)
    {
        if (realcc.find("g++") != string::npos)
        {
            dprintf("compiler language: C++ (from compiler name)\n");
            cctype = "c++";
        }
        else
        {
            if (source.substr(source.length()-2) == ".c")
            {
                dprintf("compiler language: C (from source file extension)\n");
                cctype = "c";
            }
            else
            {
                dprintf("compiler language: C++ (default)\n");
                cctype = "c++";
            }
        }
    }
    else
    {
        dprintf("compiler language: %s (from option '-x')\n", cctype.c_str());
    }

    if (cctype == "c")
    {
        hashtype = "unsigned long";
    }
    else
    {
        hashtype = "trace_hash_val";
    }
    // define TRAC_TRACEPP for macros
    tmp = "-DTRAC_TRACEPP -DTRAC_PPVER=";
    tmp += macro_version;
    cppopts.push_back(tmp);
    if (dodeps)
    {
        if (depfile.empty())
        {
            if ((p_env = getenv("DEPENDENCIES_OUTPUT")) != NULL)
            {
                depfile = p_env;
            }
            else if ((p_env = getenv("SUNPRO_DEPENDENCIES")) != NULL)
            {
                depfile = p_env;
            }
            else
            {
                depfile = object;
                if (depfile.substr(depfile.length()-2) == ".o")
                {
                    depfile = depfile.substr(0, depfile.length()-2);
                    depfile += ".d";
                }
            }
        }
        tmp = "-MD -MF ";
        tmp += depfile;
        cppopts.push_back(tmp);
    }

    //------------------------------------------------------------------------------
    // start cpp
    //------------------------------------------------------------------------------
    cmd = realcpp;
    for(vector<string>::iterator p = cppopts.begin(); p != cppopts.end(); ++p)
    {
        cmd += " ";
        cmd += *p;
    }
    cmd += " ";
    cmd += source;
    cmd += " -o-";
    dprintf("starting as cpp '%s'\n", cmd.c_str());
    CPP = popen(cmd.c_str(), "r");
    if (CPP == NULL)
    {
        fprintf(stderr, "cannot start cpp '%s'\n", realcpp.c_str());
        perror("");
        exit(1);
    }

    //------------------------------------------------------------------------------
    // start cc. manually set language as source file extension not available to cc
    //------------------------------------------------------------------------------
    string type_str = "";
    if (!optx_found)
    {
        // no option -x given by caller, set manually
        type_str = "-x ";
        type_str += cctype;
    }
    cmd = realcc;
    cmd += " ";
    cmd += type_str;
    for(vector<string>::iterator p = ccopts.begin(); p != ccopts.end(); ++p)
    {
        cmd += " ";
        cmd += *p;
    }
    cmd += " -o ";
    cmd += object;
    dprintf("starting as cc '%s'\n", cmd.c_str());
    CC = popen(cmd.c_str(), "w");
    if (CC == NULL)
    {
        fprintf(stderr, "cannot start cc '%s'\n", realcc.c_str());
        perror("");
        exit(1);
    }

    string modifiedfile;
    string unmodifiedfile;
    if (debug)
    {
        modifiedfile = object + ".debug";
        DEBUG = fopen(modifiedfile.c_str(), "w");
        if (DEBUG == NULL)
        {
            string msg = "cannot open file ";
            msg += modifiedfile;
            perror(msg.c_str());
            modifiedfile = "";
        }
        else
        {
            fprintf(stderr, "writing preprocessed source to %s\n", modifiedfile.c_str());
        }
        unmodifiedfile = object + ".debug_in";
        DEBUGIN = fopen(unmodifiedfile.c_str(), "w");
        if (DEBUGIN == NULL)
        {
            string msg = "cannot open file ";
            msg += unmodifiedfile;
            perror(msg.c_str());
            unmodifiedfile = "";
        }
        else
        {
            fprintf(stderr, "writing unprocessed source to %s\n", unmodifiedfile.c_str());
        }
    }

    string oldline;
    string newline;
    static const int MAX_BUFFER = 51200;
    char buf[MAX_BUFFER];
    while (!feof(CPP))
    {
        if (fgets(buf, MAX_BUFFER, CPP) != NULL)
        {
            oldline = buf;
            if (DEBUGIN) { fprintf(DEBUGIN, "%s", oldline.c_str()); }
            parse_line(hashtab, oldline, newline);
            //#print "oldline = $oldline";
            //#print "newline = $newline";
            if (newline.empty())
            {
                fprintf(stderr, "hash error in/with file %s\n", source.c_str());
                exit(1);
            }
            //#print "newline = $newline\n";
            fprintf(CC, "%s", newline.c_str());
            if (DEBUG) { fprintf(DEBUG, "%s", newline.c_str()); }
        }
    }
    if (DEBUG) { fclose(DEBUG); }
    if (DEBUGIN) { fclose(DEBUGIN); }
    int cmd_rc = pclose(CPP);
    if (cmd_rc)
    {
        fprintf(stderr, "error from cpp\n");
        if (cmd_rc & 127)
        {
            fprintf(stderr, "cpp got signal %d\n", (cmd_rc & 127));
            exit(1);
        }
        else if (cmd_rc >> 8)
        {
            fprintf(stderr, "cpp returned %d\n", (cmd_rc >> 8));
            exit(cmd_rc >> 8);
        }
    }
    cmd_rc = pclose(CC);
    if (cmd_rc)
    {
        fprintf(stderr, "error from cc\n");
        if (cmd_rc & 127)
        {
            fprintf(stderr, "cc got signal %d\n", (cmd_rc & 127));
            exit(1);
        }
        else if (cmd_rc >> 8)
        {
            fprintf(stderr, "cc returned %d\n", (cmd_rc >> 8));
            exit(cmd_rc >> 8);
        }
    }

    if (!hashtab.empty())
    {
        string stringfile = object;
        stringfile += ".hash";
        // open trace string file
        FILE* TRC = fopen(stringfile.c_str(), "w");
        if (TRC == NULL)
        {
            fprintf(stderr, "cannot write trace string file '%s'\n", stringfile.c_str());
            exit(1);
        }
        dprintf("Writing to file %s\n", stringfile.c_str());

        string pwd;
        FILE* PWD = popen("pwd", "r");
        fgets(buf, MAX_BUFFER, PWD);
        pwd = buf;
        pclose(PWD);
        time_t tt = time(NULL);
        sprintf(buf, "%s", asctime(localtime(&tt)));
        buf[strlen(buf)-1] = '\0'; // chop off extra newline
        fprintf(TRC, "#FSP_TRACE_v2|||%s|||BUILD:%s", buf, pwd.c_str());

        string srch_str = "||";
        srch_str += source;
        int srch_str_len = srch_str.length();
        size_t pos;
        for(map<string,string>::iterator p = hashtab.begin(); p != hashtab.end(); ++p)
        {
            pos = (p->second).find(srch_str);
            if ((pos != string::npos) && ((pos + srch_str_len) == (p->second).length()))
            {
                // source file name is already part of the string
                fprintf(TRC, "%s||%s\n", (p->first).c_str(), (p->second).c_str());
            }
            else
            {
                fprintf(TRC, "%s||%s||%s\n", (p->first).c_str(), (p->second).c_str(), source.c_str());
            }
            //#print TRC "$key||$source||$hashtab{$key}\n";
        }
        fclose(TRC);
    }
    else
    {
        dprintf("No trace calls/strings found, not writing hash file\n");
    }
} // end main
