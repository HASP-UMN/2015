// write_photon_data.c
// ISA bus transaction to read port 0x800 - 0x
// 06-10-15

#include <stdio.h>
#include <sys/io.h>
#include <unistd.h>
#include <irqreturn.h>
#include <linux/interrupt.h>

#include "globaldefs.h"

//PORT ADDRESS ON THE ISA BUS
const unsigned short INPUT_PORT = 0x800;
int SYNC_BYTE = 77; //arbitrarily chosen for now
int IRQ = 6;

static irqreturn_t write_photon_data(int irq, void *dev_id, struct pt_regs *regs)
{
    
    //SET PERMISSION TO ACCESS THE PORT ADDRESS
    fprintf(stderr,"SETTING PERMISSION TO ACCESS PORT AT 0x%x - 0x%lx",INPUT_PORT,INPUT_PORT+LENGTH);
    
    if(ioperm(INPUT_PORT,LENGTH+1,1)) {
        perror("ERROR");
        fprintf(stderr,"UNABLE TO SET PERMISSION TO ACCESS 0x%x - 0x%lx",INPUT_PORT,INPUT_PORT+LENGTH);
        return -1;
    }//dont think the permissions need to be set in this function. Seems like they should only have to be set once in main()
    
    //READ LENGTH BYTES FROM THE INPUT_PORT AT INPUT_ADRESS
    insb(INPUT_PORT,PHOTON_DATA_BUFFER,LENGTH);

    PHOTONS_AQUIRED = 1; //set flag so store_data() can be called immediately after
    
    //STORE DATA IN FILE
    return IRQ_HANDLED;
}

int store_data(){
    
    FILE* photonsA;
    FILE* photonsB;
    FILE* photonsC;
    FILE* photonsD;
    unsigned char write_buf[BYTES_PER_PHOTON];
    int i;
    
    for (i = 0; i<LENGTH; i+=10) {
        
        if ( (int)PHOTON_DATA_BUFFER[i] != SYNC_BYTE){
            fprintf(stderr, "Photon data sync byte wrong: exiting store_data.c\n");
            return -1;
        }
        
        switch (PHOTON_DATA_BUFFER[i+1]) {//detector number comes after SYNC byte
            case 1:
                photonsA = fopen("/home/root/peakLogging/eventACounts", "a");
                fwrite(PHOTON_DATA_BUFFER + i, 1, BYTES_PER_PHOTON, photonsA);
                fclose(photonsA);
                
                break;
            
            case 2:
                photonsB = fopen("/home/root/peakLogging/eventBCounts", "a");
                fwrite(PHOTON_DATA_BUFFER + i, 1, BYTES_PER_PHOTON, photonsB);
                break;
            
            case 3:
                photonsC = fopen("/home/root/peakLogging/eventCCounts", "a");
                fwrite(PHOTON_DATA_BUFFER + i, 1, BYTES_PER_PHOTON, photonsC);
                
            case 4:
                photonsD = fopen("/home/root/peakLogging/eventDCounts", "a");
                fwrite(PHOTON_DATA_BUFFER + i, 1, BYTES_PER_PHOTON, photonsD);
                
            default:
                fprintf(stderr, "Hit defualt case in store_data.c. Data not stored. exiting");
                return -1;
                break;
        }
    }
}

