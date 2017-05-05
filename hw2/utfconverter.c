#include "utfconverter.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
//#include "utfconverter.h"

#ifdef CSE320
    #define cse320(msg) printf("CSE320: HOSTNAME: %s \n",msg)
    #define cse3201(msg,msg1,msg2,msg3) printf("CSE320: INPUT FILE : %s , %lu, %lu ,Bytes %lu \n",msg,msg1,msg2,msg3)
    //#define cse3201(msg, msg1,msg2,msg3) printf("CSE20:INPUT FILE: %s , %lu ,%lu ,%lu\n")
    #define cse3202(msg) printf("CSE320:OUTPUT FILE: %s \n ",msg)
    #define cse3203(msg) printf("INPUT ENCODING: %s \n",msg)
    #define cse3204(msg) printf("OUTPUT ENCODING: %s \n",msg)
#else
    #define cse320(msg)
    #define cse3201(msg,msg1,msg2,msg3)
  //  #define cse3201(msg, msg1,msg2,msg3)
   #define cse3202(msg)
   #define cse3203(msg)
   #define cse3204(msg)
#endif



int main(int argc, char *argv[]){
    int opt, return_code = EXIT_FAILURE;
    char *input_path = NULL;
    char *output_path = NULL;
    // we use this integer to check if the code is complie on a big endian machine or a small endian machine
    int n=1;
    int vflag=0;
    int eflag=0;
    char hostname[128];
    gethostname(hostname,sizeof(hostname));
    cse320(hostname);
    //cse3201("123");



    /* open output channel */
    FILE* standardout = fopen("stdout", "w");

    /* Parse short options */
    while((opt = getopt(argc, argv, "hv::e:")) != -1) {
        switch(opt) {
            case 'h':
                /* The help menu was selected */
                USAGE(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'v':
            if(optarg !=0){
              //v_opt_arg = optarg;
              if(strcmp(optarg, "vv") == 0){
                vflag=vflag+3;
              }else if((strcmp(optarg, "v"))==0){
               vflag=vflag+2;
             }
          }else{
              vflag++;
            }
            break;
          case 'e':
              if(strcmp(optarg,"UTF-8")==0){
                eflag =1;
              }else if (strcmp(optarg,"UTF-16LE")==0){
                eflag =2;
              }else if (strcmp(optarg,"UTF-16BE")==0){
                eflag =3;
              }else{
                eflag =4;
              }
              if( eflag >=4 ){
                printf("The flag '-e' has an incorrect value %s \n",optarg);
                USAGE(argv[0]);
                exit(EXIT_FAILURE);
                break;
              }

              break;
            case '?':
                /* Let this case fall down to default;
                 * handled during bad option.*//*
                 */
            default:
                /* A bad option was provided. */
                //printf("Missing\n");
                USAGE(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }



  //  printf("number of v %d\n", vflag);




    /* Get position arguments */
    if(optind < argc && (argc - optind) == 2) {
        input_path = argv[optind++];
        output_path = argv[optind++];


        if(eflag == 0){
          printf("Missing the -e flag. This flag is required\n");
          printf("The file %s was not created\n",output_path);
          USAGE(argv[0]);
          exit(EXIT_FAILURE);
        }


    } else {
        if((argc - optind) <= 0) {
            fprintf(standardout, "Missing INPUT_FILE and OUTPUT_FILE.\n");
        } else if((argc - optind) == 1) {
            fprintf(standardout, "Missing OUTPUT_FILE.\n");
        } else {
            fprintf(standardout, "Too many arguments provided.\n");
        }
        USAGE(0[argv]);
        exit(EXIT_FAILURE);
    }
    /* Make sure all the*/// arguments were provided */
    if(input_path != NULL || output_path != NULL) {
        int input_fd = -1, output_fd = -1;
        bool success = false;
        switch(validate_args(input_path, output_path)) {
                case VALID_ARGS:
                    /* Attempt to open the input file */
                    if((input_fd = open(input_path, O_RDONLY)) < 0) {
                        fprintf(standardout, "Failed to open the file %s\n", input_path);
                        perror(NULL);
                        goto conversion_done;
                    }
                    /* Delete the output file if it exists; Don't care about return code. */
                    unlink(output_path);
                    /* Attempt to create the file */
                    if((output_fd = open(output_path, O_CREAT | O_WRONLY,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0) {
                        /* Tell the user that the file failed to be created */
                        fprintf(standardout, "Failed to open the file %s\n", input_path);
                        perror(NULL);
                        goto conversion_done;
                    }
                    /* Start the conversion */
                    //yin if it is a little endian machine else big e machine



                    cse3203("UTF-8");
                    if(eflag==1||eflag==0){
                     cse3204("UTF-8");
                    }else if(eflag==2){
                     cse3204("UTF-16LE");
                    }else if(eflag==3){
                    cse3204("UTF-16BE");
                    }







                    if(*(char *)&n ==1 ){
                    //  printf("enter smalll");


                    success = convert(input_fd, output_fd,vflag,eflag);
                  }else{
                  //  printf("enter big");
                    success = convertBig(input_fd,output_fd);
                  }
conversion_done:
                    if(success) {
                        /* We got here so it must of worked right? */
                        return_code = EXIT_SUCCESS;
                        printf("The file %s was successfully created\n",output_path);

                        break;
                    } else {
                        /* Conversion failed; clean up */
                        if(output_fd < 0 && input_fd >= 0) {
                            close(input_fd);
                        }
                        if(output_fd >= 0) {
                            close(output_fd);
                            unlink(output_path);
                        }
                        /* Just being pedantic... */
                        return_code = EXIT_FAILURE;
                    }
                case SAME_FILE:
                    fprintf(standardout, "The output file %s was not created. Same as input file.\n", output_path);
                    break;
                case FILE_DNE:
                    fprintf(standardout, "The input file %s does not exist.\n", input_path);
                    break;
                default:
                    fprintf(standardout, "An unknown error occurred\n");
                    return return_code;
                    //continue;
        }
    } else {
        /* Alert the user*/// what was not set before quitting. */
        if((input_path = NULL) == NULL) {
            fprintf(standardout, "INPUT_FILE was not set.\n");
        }
        if((output_path = NULL) == NULL) {
            fprintf(standardout, "OUTPUT_FILE was not set.\n");
        }
        // Print out the program usage
        USAGE(argv[0]);
    }
    return return_code;
}

int validate_args(const char *input_path, const char *output_path){
    int return_code = FAILED;
    /* number of arguments */
    int vargs = 2 //*
    			  //*/ 2
    			;
    struct stat firstfile;
    stat(input_path,&firstfile);
    struct stat secondfile;
    stat(output_path,&secondfile);
    cse3201(input_path, firstfile.st_dev,firstfile.st_ino,firstfile.st_size );
    cse3202( output_path);
    //char *vargs[2] = [*input_path,*input_path];
    /* create reference */
    void* pvargs = &vargs;
    /* Make sure both strings are not NULL */
    if(input_path != NULL && output_path != NULL) {
        /* Check to see if the the input and output are two different files. */
      //  printf("firstfile device %lu",firstfile.st_dev);
      //  printf("firstfile inp %lu",firstfile.st_ino);
      //  printf("second device %lu",secondfile.st_dev);
      //  printf("second inp %lu",secondfile.st_ino);

        if((firstfile.st_dev == secondfile.st_dev && firstfile.st_ino == secondfile.st_ino) == 0) {
            /* Check to see if the input file exists */
            struct stat sb;
            /* zero out the memory of one sb plus another */
            memset(&sb, 0, sizeof(sb) + 1);
            /* increment to second argument */
            pvargs++;
            /* now check to see if the file exists */
            if(stat(input_path, &sb) == -1) {
                /* something went wrong */
                if(errno == ENOENT) {
                    /* File does not exist. */
                    return_code = FILE_DNE;
                } else {
                    /* No idea what the error is. */
                    perror("NULL");
                }

            } else {
                return_code = VALID_ARGS;
            }
        }else{
          printf("The file %s was not created.Same as input file\n",output_path);
          exit(EXIT_FAILURE);
        }
    }
    /* Be good and free memory */

    //free(pvargs);
    //pvargs = NULL;
    return return_code;
}

bool convert(const int input_fd, const int output_fd, int vflag, int eflag){
    bool success = false;
    if(input_fd >= 0 && output_fd >= 0) {
        /* UTF-8 encoded text can be @ most 4-bytes */
        //unsigned char bytes['4'-'0'];
       unsigned char bytes[4]="";
        auto unsigned char read_value;
        auto size_t count = 0;
        auto int safe_param = SAFE_PARAM;// DO NOT DELETE, PROGRAM WILL BE UNSAFE //
        void* saftey_ptr = &safe_param;
        auto ssize_t bytes_read;
        bool encode = false;
        bool noascii = true;
        bool printedvhead = false;
        FILE* standarderror = fopen("stderr","w");
        bool printedbom = false;









        /* Read in UTF-8 Bytes */
        while((bytes_read = read(input_fd, &read_value, 1)) == 1) {













          unsigned char masked_value = read_value & 0x80;
      //    printf( " read value %d \n",read_value);
          if(masked_value == 0x80) {
            if((read_value & UTF8_4_BYTE) == UTF8_4_BYTE ||
                   (read_value & UTF8_3_BYTE) == UTF8_3_BYTE ||
                   (read_value & UTF8_2_BYTE) == UTF8_2_BYTE) {
                     if(count == 0) {
                       bytes[count++] = read_value;
                     }else {
                         if(lseek(input_fd, -1, SEEK_CUR) < 0) {
                         	safe_param = *(int*)++saftey_ptr;
                             perror("NULL");
                             goto conversion_done;
                         }
                         encode = true;
                     }
                   }else if((read_value & UTF8_CONT) == UTF8_CONT) {
                       bytes[count++] = read_value;
                   }
          } else {
              if(count == 0) {
                  /* US-ASCII */
                  bytes[count++] = read_value;
                  encode = true;
                  noascii = false;
              } else{
                  if(lseek(input_fd, -1, SEEK_CUR) < 0) {
                    /*Unsafe action! Increment! */
                      safe_param = *(int*) ++saftey_ptr;
                      /* failed to move the file pointer back */
                      perror("NULL");
                      goto conversion_done;
                  }
                  encode = true;
                }
        }


        if(encode) {
            int i, value = 0;
            bool isAscii = false;
            for(i=0; i < count; i++) {
                if(i == 0) {
                    if((bytes[i] & UTF8_4_BYTE) == UTF8_4_BYTE) {
                        value = bytes[i] & 0x7;
                    } else if((bytes[i] & UTF8_3_BYTE) == UTF8_3_BYTE) {
                        value =  bytes[i] & 0xF;
                    } else if((bytes[i] & UTF8_2_BYTE) == UTF8_2_BYTE) {
                        value =  bytes[i] & 0x1F;
                    } else if((bytes[i] & 0x80) == 0) {
                        /* Value is an ASCII character */
                        value = bytes[i];
                        isAscii = true;
                    } else {
                        /* Marker byte is incorrect */
                      //  printf("Marker byte is incorrect ");
                        goto conversion_done;
                    }
                } else {
                    if(!isAscii) {
                        value = (value << 6) | (bytes[i] & 0x3F);
                    } else {
                        /* How is there more// bytes if we have an ascii char? */
                      //  printf("enter done");
                        goto conversion_done;
                    }
                }
            }
// at this point we have the codepoint





//////////////////////for eflag ==

///////add BOM file
if(printedbom == false){
          if(eflag==0||eflag ==2){
            int bomff = 255;
            int bomfe = 254;
            if(!safe_write(output_fd, &bomff, CODE_UNIT_SIZE)) {
    /* Assembly for some super efficient coding */
          goto conversion_done;
              }
      if(!safe_write(output_fd, &bomfe, CODE_UNIT_SIZE)) {
/* Assembly for some super efficient coding */
          goto conversion_done;
              }

          }else if(eflag == 1){
            int bomef = 239;
            int bombb = 187;
            int bombf = 191;
            if(!safe_write(output_fd, &bomef, CODE_UNIT_SIZE)) {
    /* Assembly for some super efficient coding */
                  goto conversion_done;
              }
            if(!safe_write(output_fd, &bombb, CODE_UNIT_SIZE)) {
      /* Assembly for some super efficient coding */
                    goto conversion_done;
              }
            if(!safe_write(output_fd, &bombf, CODE_UNIT_SIZE)) {
        /* Assembly for some super efficient coding */
                      goto conversion_done;
                }
          }else if(eflag ==3){
            int bomfe = 254;
            int bomff = 239;
            if(!safe_write(output_fd, &bomfe, CODE_UNIT_SIZE)) {
      /* Assembly for some super efficient coding */
                    goto conversion_done;
              }
            if(!safe_write(output_fd, &bomff, CODE_UNIT_SIZE)) {
        /* Assembly for some super efficient coding */
                      goto conversion_done;
                }

          }

          printedbom = true;
}










            if(value >= SURROGATE_PAIR) {
                int vprime = value - SURROGATE_PAIR;
                int w1 = (vprime >> 10) + 0xD800;
                int w2 = (vprime & 0x3FF) + 0xDC00; /*(vprime & 0x3FF) + 0xDC00*/;
                /* write the surrogate pai*//*r to file */
              //  printf("w111111111 %d", w1);
                //printf("w122222222222 %d", w2);
                if(w1>255 || w2>255){
                    if(w1>255){
                        int w11 = w1 & 0xFF;
                        int w12 = w1 & 0xFF00;
                        w12 = w12 >> 8;
                          if(!safe_write(output_fd, &w11, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
                    }

                if(!safe_write(output_fd, &w12, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
                }
              }else{
                if(!safe_write(output_fd, &w1, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
              }
            }

              if(w2>255){

                int w21 = w2 & 0xFF;
                int w22 = w2 & 0xFF00;
                w22 = w22 >> 8;
                if(!safe_write(output_fd, &w21, CODE_UNIT_SIZE)) {

                  goto conversion_done;
                }

                if(!safe_write(output_fd, &w22, CODE_UNIT_SIZE)) {
                /* Assembly for some super efficient coding */

                  goto conversion_done;
                }
              }else{
                if(!safe_write(output_fd, &w2, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
              }
            }




}else{
              //  printf("write %d \n",w1);
                if(!safe_write(output_fd, &w1, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
                }
                printf("write %d \n",w2);
                if(!safe_write(output_fd, &w2, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
                }
              }
            } else {
                /* write the code point to file */

                if(value > 255 ){
                  int w1 = value & 0xFF;
                  int w2 = value & 0xFF00;
                  w2 =w2 >> 8;
                //  printf("w1 %d\n", w1);
                //  printf("w2 %d\n", w2);
                  if(!safe_write(output_fd, &w1, CODE_UNIT_SIZE)) {
                    /* Assembly for some super efficient coding */

                      goto conversion_done;
                  }
                  if(!safe_write(output_fd, &w2, CODE_UNIT_SIZE)) {
                    /* Assembly for some super efficient coding */

                      goto conversion_done;
                  }
                }else{
                int w0=0;




                if(vflag==1 && printedvhead == 0){
                  printf("+--------------+----------------+-------------------------+\n");
                  printf("    ASCII      |   # of bytes   |          codepoint      |\n");
                  fprintf(standarderror, "+--------------+----------------+-------------------------+\n");
                  fprintf(standarderror, "    ASCII      |   # of bytes   |          codepoint      |\n");
                  printedvhead = true;
                }else if(vflag==2 && printedvhead == 0){
                  printf("+--------------+----------------+-------------------------+------------------\n");
                  printf("    ASCII      |   # of bytes   |          codepoint      |      input       \n");
                  fprintf(standarderror, "+--------------+----------------+-------------------------+------------------\n");
                  fprintf(standarderror, "    ASCII      |   # of bytes   |          codepoint      |      input       \n");
                  printedvhead = true;
                }else if(vflag==3 && printedvhead == 0) {
                  printf("+--------------+----------------+-------------------------+------------------+-------------------\n");
                  printf("    ASCII      |   # of bytes   |          codepoint      |      input       |      output       \n");
                  fprintf(standarderror, "+--------------+----------------+-------------------------+------------------+-------------------\n");
                  fprintf(standarderror, "    ASCII      |   # of bytes   |          codepoint      |      input       |      output       \n");
                  printedvhead = true;
                }


                if(noascii != 0){
                  printf("NONE\n");
                }


                if(vflag == 1 && noascii == 0){

                printf("+--------------+----------------+-------------------------+\n");
                fprintf(standarderror, "+--------------+----------------+-------------------------+\n");
                if(value !=10 && value >=15){
                printf("     %c        |      1         |          U+0x00%X         |\n", value,value);
                fprintf(standarderror,"     %c        |      1         |          U+0x00%X         |\n", value,value);
              }else if(value ==10){
                printf("     \\n        |      1         |          U+0x000%X         |\n",value);
                fprintf(standarderror,"     \\n        |      1         |          U+0x000%X         |\n",value);
              }else{
                printf("     %c        |      1         |          U+0x000%X         |\n", value,value);
                fprintf(standarderror,"     %c        |      1         |          U+0x000%X         |\n", value,value);
              }
            }else if(vflag == 2 && noascii == 0){

              printf("+--------------+----------------+----------  ---------------+---------------------\n");
              fprintf(standarderror,"+--------------+----------------+----------  ---------------+---------------------\n");
              if(value !=10 && value >=15){
              printf("     %c        |      1         |          U+0x00%X         |      0x%X           |\n", value,value,value);
              fprintf(standarderror,"     %c        |      1         |          U+0x00%X         |      0x%X           |\n", value,value,value);
            }else if(value ==10){
              printf("     \\n        |      1         |          U+0x000%X       |      0x%X           |\n",value,value);
              fprintf(standarderror,"     \\n        |      1         |          U+0x000%X       |      0x%X           |\n",value,value);
            }else{
              printf("     %c        |      1         |          U+0x000%X        |      0x%X           |\n", value,value,value);
              fprintf(standarderror,"     %c        |      1         |          U+0x000%X        |      0x%X           |\n", value,value,value);
            }

          }else{
            printf("+--------------+----------------+----------  ---------------+-------------------|-------------------|\n");
            fprintf(standarderror,"+--------------+----------------+----------  ---------------+-------------------|-------------------|\n");
            if(value !=10 && value >=15){
            printf("     %c        |      1         |          U+0x00%X         |      0x%X           |     0x00%X        |\n", value,value,value,value);
              fprintf(standarderror,"     %c        |      1         |          U+0x00%X         |      0x%X           |     0x00%X        |\n", value,value,value,value);
          }else if(value ==10){
            printf("     \\n        |      1         |          U+0x000%X       |      0x%X           |     0x000%X        |\n",value,value,value);
            fprintf(standarderror,"     \\n        |      1         |          U+0x000%X       |      0x%X           |     0x000%X        |\n",value,value,value);
          }else{
            printf("     %c        |      1         |          U+0x000%X        |      0x%X           |     0x000%X        |\n", value,value,value,value);
            fprintf(standarderror,"     %c        |      1         |          U+0x000%X        |      0x%X           |     0x000%X        |\n", value,value,value,value);
          }

          }





                if(!safe_write(output_fd, &value, CODE_UNIT_SIZE)) {
                  /* Assembly *///for some super efficient coding */

                    goto conversion_done;
                }


                if(!safe_write(output_fd, &w0, CODE_UNIT_SIZE)) {
                  /* Assembly *///for some super efficient coding */

                    goto conversion_done;
                }
}












            }
            /* Done encoding the*/// value to UTF-16LE */
            encode = false;
            count = 0;





          }

   /*<-------------------------------if eflag ==0*/
    //after encode code
  }/*<-----------------------------while loop*/

        /* If we got here the operation was a success! */
        success = true;



        if(vflag == 1){
          printf("+--------------+----------------+-------------------------+\n");
          fprintf(standarderror,"+--------------+----------------+-------------------------+\n");
        }else if(vflag == 2){
          printf("+--------------+----------------+----------  ---------------+---------------------\n");
          fprintf(standarderror,"+--------------+----------------+----------  ---------------+---------------------\n");
        }else if(vflag ==3){
          printf("+--------------+----------------+----------  ---------------+-------------------|-------------------|\n");
          fprintf(standarderror,"+--------------+----------------+----------  ---------------+-------------------|-------------------|\n");
        }







    }
conversion_done:
    return success;
}




bool safe_write(int output_fd, void *value, size_t size){
    bool success = true;
    ssize_t bytes_written;
    if((bytes_written = write(output_fd, value, size)) != size) {
        /* The write operation failed */
        fprintf(stdout, "Write to file failed. Expected %zu bytes but got %zd\n", size, bytes_written);
    }
    return success;
}


















bool convertBig(const int input_fd, const int output_fd){
    bool success = false;
    if(input_fd >= 0 && output_fd >= 0) {
        /* UTF-8 encoded text can be @ most 4-bytes */
        //unsigned char bytes['4'-'0'];
       unsigned char bytes[4]="";
        auto unsigned char read_value;
        auto size_t count = 0;
        auto int safe_param = SAFE_PARAM;// DO NOT DELETE, PROGRAM WILL BE UNSAFE //
        void* saftey_ptr = &safe_param;
        auto ssize_t bytes_read;
        bool encode = false;
        /* Read in UTF-8 Bytes */
        while((bytes_read = read(input_fd, &read_value, 1)) == 1) {
          unsigned char masked_value = read_value & 0x80;
        //  printf( " read value %d \n",read_value);
          if(masked_value == 0x80) {
            if((read_value & UTF8_4_BYTE) == UTF8_4_BYTE ||
                   (read_value & UTF8_3_BYTE) == UTF8_3_BYTE ||
                   (read_value & UTF8_2_BYTE) == UTF8_2_BYTE) {
                     if(count == 0) {
                       bytes[count++] = read_value;
                     }else {
                         if(lseek(input_fd, -1, SEEK_CUR) < 0) {
                         	safe_param = *(int*)++saftey_ptr;
                             perror("NULL");
                             goto conversion_done;
                         }
                         encode = true;
                     }
                   }else if((read_value & UTF8_CONT) == UTF8_CONT) {
                       bytes[count++] = read_value;
                   }
          } else {
              if(count == 0) {
                  /* US-ASCII */
                  bytes[count++] = read_value;
                  encode = true;
              } else{
                  if(lseek(input_fd, -1, SEEK_CUR) < 0) {
                    /*Unsafe action! Increment! */
                      safe_param = *(int*) ++saftey_ptr;
                      /* failed to move the file pointer back */
                      perror("NULL");
                      goto conversion_done;
                  }
                  encode = true;
                }
        }
        if(encode) {
            int i, value = 0;
            bool isAscii = false;
            for(i=0; i < count; i++) {
                if(i == 0) {
                    if((bytes[i] & UTF8_4_BYTE) == UTF8_4_BYTE) {
                        value = bytes[i] & 0x7;
                    } else if((bytes[i] & UTF8_3_BYTE) == UTF8_3_BYTE) {
                        value =  bytes[i] & 0xF;
                    } else if((bytes[i] & UTF8_2_BYTE) == UTF8_2_BYTE) {
                        value =  bytes[i] & 0x1F;
                    } else if((bytes[i] & 0x80) == 0) {
                        /* Value is an ASCII character */
                        value = bytes[i];
                        isAscii = true;
                    } else {
                      /* Marker byte is incorrect */
                      //  printf("Marker byte is incorrect ");
                        goto conversion_done;
                    }
                } else {
                    if(!isAscii) {
                        value = (value << 6) | (bytes[i] & 0x3F);
                    } else {
                        /* How is there more// bytes if we have an ascii char? */
                      //  printf("enter done");
                        goto conversion_done;
                    }
                }
            }
          //  printf("value%d\n",value);
            if(value >= SURROGATE_PAIR) {
                int vprime = value - SURROGATE_PAIR;
                int w1 = (vprime >> 10) + 0xD800;
                int w2 = (vprime & 0x3FF) + 0xDC00; /*(vprime & 0x3FF) + 0xDC00*/;
                /* write the surrogate pai*//*r to file */
              //  printf("w111111111 %d", w1);
                //printf("w122222222222 %d", w2);
                if(w1>255 || w2>255){
                    if(w2>255){
                        int w21 = w2 & 0xFF;
                        int w22 = w2 & 0xFF00;
                        w22 = w22 >> 8;
                      //  printf("write %d \n",w22);
                        w22 = reverseInt(w22);
                          if(!safe_write(output_fd, &w22, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
                    }
                      //  printf("write %d \n",w21);
                        w21 = reverseInt(w21);
                if(!safe_write(output_fd, &w21, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
                }
              }else{
                w2 = reverseInt(w2);
                if(!safe_write(output_fd, &w2, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
              }
            }

              if(w1>255){

                int w11 = w1 & 0xFF;
                int w12 = w1 & 0xFF00;
                w12 = w12 >> 8;
              //  printf("write %d \n",w12);

                w12 = reverseInt(w12);
                if(!safe_write(output_fd, &w12, CODE_UNIT_SIZE)) {

                  goto conversion_done;
                }
              //  printf("write %d \n",w11);
                w11 = reverseInt(w11);
                if(!safe_write(output_fd, &w11, CODE_UNIT_SIZE)) {
                /* Assembly for some super efficient coding */

                  goto conversion_done;
                }
              }else{
                w1 = reverseInt(w1);
                if(!safe_write(output_fd, &w1, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
              }
            }




}else{
            //    printf("write %d \n",w2);
                w2 = reverseInt(w2);
                w1 = reverseInt(w1);
                if(!safe_write(output_fd, &w2, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
                }
              //  printf("write %d \n",w1);
                if(!safe_write(output_fd, &w1, CODE_UNIT_SIZE)) {
                  /* Assembly for some super efficient coding */

                    goto conversion_done;
                }
              }
            } else {
                /* write the code point to file */

                if(value > 255 ){
                  int w1 = value & 0xFF;
                  int w2 = value & 0xFF00;
                  w2 =w2 >> 8;

                /////  printf("w1 %d", w1);
                //  printf("w2 %d\n", w2);
/*
                  int firsthalf = w1 & 0xF0;
                  int secondhalf = w1 & 0xF;
                  firsthalf = firsthalf >> 4;
                  secondhalf = secondhalf <<4;
                  w1 = firsthalf + secondhalf;
                  int firsthalf2 = w2 & 0xF0;
                  int secondhalf2 = w2 & 0xF;
                  firsthalf2 = firsthalf2 >> 4;
                  secondhalf2 = secondhalf2 <<4;
                  w2 = firsthalf2 + secondhalf2;



                  int w3 =255;
                  if(!safe_write(output_fd, &w3, CODE_UNIT_SIZE)) {
 */

                  w1 = reverseInt(w1);
                  w2 = reverseInt(w2);

              //    printf("w1cv %d", w1);
              //    printf("w2cv %d\n", w2);
                  if(!safe_write(output_fd, &w2, CODE_UNIT_SIZE)) {
                    /* Assembly for some super efficient coding */

                      goto conversion_done;
                  }
                  if(!safe_write(output_fd, &w1, CODE_UNIT_SIZE)) {
                    /* Assembly for some super efficient coding */

                      goto conversion_done;
                  }
                }else{
                int w0=0;
                if(!safe_write(output_fd, &w0, CODE_UNIT_SIZE)) {
                  /* Assembly *///for some super efficient coding */

                    goto conversion_done;
                }

                value = reverseInt(value);

                if(!safe_write(output_fd, &value, CODE_UNIT_SIZE)) {
                  /* Assembly *///for some super efficient coding */

                    goto conversion_done;
                }
}












            }
            /* Done encoding the*/// value to UTF-16LE */
            encode = false;
            count = 0;





          }


    //after encode code
  }

        /* If we got here the operation was a success! */
        success = true;
    }
conversion_done:
    return success;
}


int reverseInt(int i){
  unsigned char c1,c2,c3,c4;
  c1 = i & 255;
  c2 = (i >> 8) & 255;
  c3 = (i >> 16) & 255;
  c4 = (i >> 24) & 255;
  return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) +c4 ;
}
