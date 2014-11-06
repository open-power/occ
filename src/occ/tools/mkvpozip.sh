DESC=$1

DATE=`date +'%m%d%y84S'`
VPODIR="/tmp/vpo_zip_$DESC"
USERNAME=$USER

mkdir -p $VPODIR
mkdir $VPODIR/gpe
cd $sb/occc/405
# Added 3-15-2012, Commented out 3-30-2012
#make clean && make NO_TRAC_STRINGS=1 OCC_SVN_SIMICS=1 && make combineImage  #No APSS
# Added 3-30-2012, Commented out x-xx-xxxx
#make clean && make NO_TRAC_STRINGS=1 && make combineImage  #w/ APSS
mk clobber_all && mk -a -DFSPLESS_SIMICS=1 && mk combineImage  #w/ APSS
cp $sb/../obj/ppc/occc/405/image.bin $VPODIR/occ_l3_image_$DESC.bin

# Copy OCC SRAM Image & Symbol/Disassembly
cp $sb/../obj/ppc/occc/405/occ/occ.bin $VPODIR/occ_sram_image_$DESC.bin
cp $sb/../obj/ppc/occc/405/occ/occ.out $VPODIR/occ_sram_image_$DESC.out
cp $sb/../obj/ppc/occc/405/occ/occ.map $VPODIR/occ_sram_image_$DESC.map
cp $sb/../obj/ppc/occc/405/occ/occ.dis $VPODIR/occ_sram_image_$DESC.dis

# Copy OCC Bootloader & Symbol/Disassembly
cp $sb/../obj/ppc/occc/405/occBootLoader/bootloader.map $VPODIR/occ_bootloader_$DESC.map
cp $sb/../obj/ppc/occc/405/occBootLoader/bootloader.out $VPODIR/occ_bootloader_$DESC.out
cp $sb/../obj/ppc/occc/405/occBootLoader/bootloader.dis $VPODIR/occ_bootloader_$DESC.dis

# Disassemble GPEs & Copy Disassembly of GPE into Zip
for gpeobj in $(for f in $(find ./ -iname '*\.ps'); do echo $f | sed 's/\.pS/\.o/'; done); 
do 
  BASE=`basename $gpeobj`; 
  /afs/awd/projects/cte/tools/porebinutils/prod/bin/pore-elf64-objdump -d $gpeobj > $VPODIR/gpe/$BASE.$DESC.dis; 
done

zip -r occ_vpo_$DESC.$DATE.zip $VPODIR
chmod 775 occ_vpo_$DESC.$DATE.zip

echo "Created occ_vpo_$DESC.$DATE.zip @ full path:"
echo "$sb/occ_vpo_$DESC.$DATE.zip"

rm -rf $VPODIR
#tar cvf - ./occ_vpo_$DESC.$DATE.zip | ssh $USERNAME@hdmdbld.boeblingen.de.ibm.com 'umask 0022; cd /afs/apd/u/thallet/public/occ_vpo/; tar xvf -'

echo "Do you want to copy occ_vpo_$DESC.$DATE.zip to...?"
echo "1. hdmdbld.boeblingen.de.ibm.com:/modbld/mur/tim/"
echo "2. /afs/bb/u/rembold/occ/"
echo "3. /afs/apd/u/thallet/public/occ_vpo/"
echo "4. None" 
read choice


case $choice in 
  [1]* ) #scp $sb/occ_vpo_$DESC.$DATE.zip $USERNAME@hdmdbld.boeblingen.de.ibm.com:/modbld/mur/tim/; 
         #ssh $USERNAME@hdmdbld.boeblingen.de.ibm.com chmod 755 /modbld/mur/tim/occ_vpo_$DESC.$DATE.zip
         tar cvf - ./occ_vpo_$DESC.$DATE.zip | ssh $USERNAME@hdmdbld.boeblingen.de.ibm.com 'umask 0022; cd /modbld/mur/tim/; tar xvf -'
         exit;;
  [2]* ) #scp $sb/occ_vpo_$DESC.$DATE.zip $USERNAME@hdmdbld.boeblingen.de.ibm.com:/afs/bb/u/rembold/occ/; 
         #ssh $USERNAME@hdmdbld.boeblingen.de.ibm.com chmod 755 /afs/bb/u/rembold/occ/occ_vpo_$DESC.$DATE.zip
         tar cvf - ./occ_vpo_$DESC.$DATE.zip | ssh $USERNAME@hdmdbld.boeblingen.de.ibm.com 'umask 0022; cd /afs/bb/u/rembold/occ/; tar xvf -'
         exit;;
  [3]* ) cp $sb/occ_vpo_$DESC.$DATE.zip /afs/apd/u/thallet/public/occ_vpo/; 
         chmod 755 /afs/apd/u/thallet/public/occ_vpo/occ_vpo_$DESC.$DATE.zip
         exit;;
  * ) exit;;
esac

