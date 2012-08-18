/*
Copyright (C) 2001-2011 by Serge Lamikhov-Center

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef ELFIO_DUMP_HPP
#define ELFIO_DUMP_HPP

#include <string>
#include <ostream>
#include <sstream>
#include <elfio.hpp>

namespace ELFIO {


static struct class_table_t {
    const char  key;
    const char* str;
} class_table [] = 
{
    { ELFCLASS32, "ELF32" },
    { ELFCLASS64, "ELF64" },
};


static struct endian_table_t {
    const char  key;
    const char* str;
} endian_table [] = 
{
    { ELFDATANONE, "None"          },
    { ELFDATA2LSB, "Little endian" },
    { ELFDATA2MSB, "Big endian"    },
};


static struct version_table_t {
    const Elf64_Word key;
    const char*      str;
} version_table [] = 
{
    { EV_NONE,    "None"    },
    { EV_CURRENT, "Current" },
};


static struct type_table_t {
    const Elf32_Half key;
    const char*      str;
} type_table [] = 
{
    { ET_NONE, "No file type"       },
    { ET_REL,  "Relocatable file"   },
    { ET_EXEC, "Executable file"    },
    { ET_DYN,  "Shared object file" },
    { ET_CORE, "Core file"          },
};


static struct machine_table_t {
    const Elf64_Half key;
    const char*      str;
} machine_table [] = 
{
    { EM_NONE         , "No machine"                                                              },
    { EM_M32          , "AT&T WE 32100"                                                           },
    { EM_SPARC        , "SUN SPARC"                                                               },
    { EM_386          , "Intel 80386"                                                             },
    { EM_68K          , "Motorola m68k family"                                                    },
    { EM_88K          , "Motorola m88k family"                                                    },
    { EM_486          , "Intel 80486// Reserved for future use"                                   },
    { EM_860          , "Intel 80860"                                                             },
    { EM_MIPS         , "MIPS R3000 (officially, big-endian only)"                                },
    { EM_S370         , "IBM System/370"                                                          },
    { EM_MIPS_RS3_LE  , "MIPS R3000 little-endian (Oct 4 1999 Draft) Deprecated"                  },
    { EM_res011       , "Reserved"                                                                },
    { EM_res012       , "Reserved"                                                                },
    { EM_res013       , "Reserved"                                                                },
    { EM_res014       , "Reserved"                                                                },
    { EM_PARISC       , "HPPA"                                                                    },
    { EM_res016       , "Reserved"                                                                },
    { EM_VPP550       , "Fujitsu VPP500"                                                          },
    { EM_SPARC32PLUS  , "Sun's v8plus"                                                            },
    { EM_960          , "Intel 80960"                                                             },
    { EM_PPC          , "PowerPC"                                                                 },
    { EM_PPC64        , "64-bit PowerPC"                                                          },
    { EM_S390         , "IBM S/390"                                                               },
    { EM_SPU          , "Sony/Toshiba/IBM SPU"                                                    },
    { EM_res024       , "Reserved"                                                                },
    { EM_res025       , "Reserved"                                                                },
    { EM_res026       , "Reserved"                                                                },
    { EM_res027       , "Reserved"                                                                },
    { EM_res028       , "Reserved"                                                                },
    { EM_res029       , "Reserved"                                                                },
    { EM_res030       , "Reserved"                                                                },
    { EM_res031       , "Reserved"                                                                },
    { EM_res032       , "Reserved"                                                                },
    { EM_res033       , "Reserved"                                                                },
    { EM_res034       , "Reserved"                                                                },
    { EM_res035       , "Reserved"                                                                },
    { EM_V800         , "NEC V800 series"                                                         },
    { EM_FR20         , "Fujitsu FR20"                                                            },
    { EM_RH32         , "TRW RH32"                                                                },
    { EM_MCORE        , "Motorola M*Core // May also be taken by Fujitsu MMA"                     },
    { EM_RCE          , "Old name for MCore"                                                      },
    { EM_ARM          , "ARM"                                                                     },
    { EM_OLD_ALPHA    , "Digital Alpha"                                                           },
    { EM_SH           , "Renesas (formerly Hitachi) / SuperH SH"                                  },
    { EM_SPARCV9      , "SPARC v9 64-bit"                                                         },
    { EM_TRICORE      , "Siemens Tricore embedded processor"                                      },
    { EM_ARC          , "ARC Cores"                                                               },
    { EM_H8_300       , "Renesas (formerly Hitachi) H8/300"                                       },
    { EM_H8_300H      , "Renesas (formerly Hitachi) H8/300H"                                      },
    { EM_H8S          , "Renesas (formerly Hitachi) H8S"                                          },
    { EM_H8_500       , "Renesas (formerly Hitachi) H8/500"                                       },
    { EM_IA_64        , "Intel IA-64 Processor"                                                   },
    { EM_MIPS_X       , "Stanford MIPS-X"                                                         },
    { EM_COLDFIRE     , "Motorola Coldfire"                                                       },
    { EM_68HC12       , "Motorola M68HC12"                                                        },
    { EM_MMA          , "Fujitsu Multimedia Accelerator"                                          },
    { EM_PCP          , "Siemens PCP"                                                             },
    { EM_NCPU         , "Sony nCPU embedded RISC processor"                                       },
    { EM_NDR1         , "Denso NDR1 microprocesspr"                                               },
    { EM_STARCORE     , "Motorola Star*Core processor"                                            },
    { EM_ME16         , "Toyota ME16 processor"                                                   },
    { EM_ST100        , "STMicroelectronics ST100 processor"                                      },
    { EM_TINYJ        , "Advanced Logic Corp. TinyJ embedded processor"                           },
    { EM_X86_64       , "Advanced Micro Devices X86-64 processor"                                 },
    { EM_PDSP         , "Sony DSP Processor"                                                      },
    { EM_PDP10        , "Digital Equipment Corp. PDP-10"                                          },
    { EM_PDP11        , "Digital Equipment Corp. PDP-11"                                          },
    { EM_FX66         , "Siemens FX66 microcontroller"                                            },
    { EM_ST9PLUS      , "STMicroelectronics ST9+ 8/16 bit microcontroller"                        },
    { EM_ST7          , "STMicroelectronics ST7 8-bit microcontroller"                            },
    { EM_68HC16       , "Motorola MC68HC16 Microcontroller"                                       },
    { EM_68HC11       , "Motorola MC68HC11 Microcontroller"                                       },
    { EM_68HC08       , "Motorola MC68HC08 Microcontroller"                                       },
    { EM_68HC05       , "Motorola MC68HC05 Microcontroller"                                       },
    { EM_SVX          , "Silicon Graphics SVx"                                                    },
    { EM_ST19         , "STMicroelectronics ST19 8-bit cpu"                                       },
    { EM_VAX          , "Digital VAX"                                                             },
    { EM_CRIS         , "Axis Communications 32-bit embedded processor"                           },
    { EM_JAVELIN      , "Infineon Technologies 32-bit embedded cpu"                               },
    { EM_FIREPATH     , "Element 14 64-bit DSP processor"                                         },
    { EM_ZSP          , "LSI Logic's 16-bit DSP processor"                                        },
    { EM_MMIX         , "Donald Knuth's educational 64-bit processor"                             },
    { EM_HUANY        , "Harvard's machine-independent format"                                    },
    { EM_PRISM        , "SiTera Prism"                                                            },
    { EM_AVR          , "Atmel AVR 8-bit microcontroller"                                         },
    { EM_FR30         , "Fujitsu FR30"                                                            },
    { EM_D10V         , "Mitsubishi D10V"                                                         },
    { EM_D30V         , "Mitsubishi D30V"                                                         },
    { EM_V850         , "NEC v850"                                                                },
    { EM_M32R         , "Renesas M32R (formerly Mitsubishi M32R)"                                 },
    { EM_MN10300      , "Matsushita MN10300"                                                      },
    { EM_MN10200      , "Matsushita MN10200"                                                      },
    { EM_PJ           , "picoJava"                                                                },
    { EM_OPENRISC     , "OpenRISC 32-bit embedded processor"                                      },
    { EM_ARC_A5       , "ARC Cores Tangent-A5"                                                    },
    { EM_XTENSA       , "Tensilica Xtensa Architecture"                                           },
    { EM_VIDEOCORE    , "Alphamosaic VideoCore processor"                                         },
    { EM_TMM_GPP      , "Thompson Multimedia General Purpose Processor"                           },
    { EM_NS32K        , "National Semiconductor 32000 series"                                     },
    { EM_TPC          , "Tenor Network TPC processor"                                             },
    { EM_SNP1K        , "Trebia SNP 1000 processor"                                               },
    { EM_ST200        , "STMicroelectronics ST200 microcontroller"                                },
    { EM_IP2K         , "Ubicom IP2022 micro controller"                                          },
    { EM_MAX          , "MAX Processor"                                                           },
    { EM_CR           , "National Semiconductor CompactRISC"                                      },
    { EM_F2MC16       , "Fujitsu F2MC16"                                                          },
    { EM_MSP430       , "TI msp430 micro controller"                                              },
    { EM_BLACKFIN     , "ADI Blackfin"                                                            },
    { EM_SE_C33       , "S1C33 Family of Seiko Epson processors"                                  },
    { EM_SEP          , "Sharp embedded microprocessor"                                           },
    { EM_ARCA         , "Arca RISC Microprocessor"                                                },
    { EM_UNICORE      , "Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University" },
    { EM_EXCESS       , "eXcess: 16/32/64-bit configurable embedded CPU"                          },
    { EM_DXP          , "Icera Semiconductor Inc. Deep Execution Processor"                       },
    { EM_ALTERA_NIOS2 , "Altera Nios II soft-core processor"                                      },
    { EM_CRX          , "National Semiconductor CRX"                                              },
    { EM_XGATE        , "Motorola XGATE embedded processor"                                       },
    { EM_C166         , "Infineon C16x/XC16x processor"                                           },
    { EM_M16C         , "Renesas M16C series microprocessors"                                     },
    { EM_DSPIC30F     , "Microchip Technology dsPIC30F Digital Signal Controller"                 },
    { EM_CE           , "Freescale Communication Engine RISC core"                                },
    { EM_M32C         , "Renesas M32C series microprocessors"                                     },
    { EM_res121       , "Reserved"                                                                },
    { EM_res122       , "Reserved"                                                                },
    { EM_res123       , "Reserved"                                                                },
    { EM_res124       , "Reserved"                                                                },
    { EM_res125       , "Reserved"                                                                },
    { EM_res126       , "Reserved"                                                                },
    { EM_res127       , "Reserved"                                                                },
    { EM_res128       , "Reserved"                                                                },
    { EM_res129       , "Reserved"                                                                },
    { EM_res130       , "Reserved"                                                                },
    { EM_TSK3000      , "Altium TSK3000 core"                                                     },
    { EM_RS08         , "Freescale RS08 embedded processor"                                       },
    { EM_res133       , "Reserved"                                                                },
    { EM_ECOG2        , "Cyan Technology eCOG2 microprocessor"                                    },
    { EM_SCORE        , "Sunplus Score"                                                           },
    { EM_SCORE7       , "Sunplus S+core7 RISC processor"                                          },
    { EM_DSP24        , "New Japan Radio (NJR) 24-bit DSP Processor"                              },
    { EM_VIDEOCORE3   , "Broadcom VideoCore III processor"                                        },
    { EM_LATTICEMICO32, "RISC processor for Lattice FPGA architecture"                            },
    { EM_SE_C17       , "Seiko Epson C17 family"                                                  },
    { EM_TI_C6000     , "Texas Instruments TMS320C6000 DSP family"                                },
    { EM_TI_C2000     , "Texas Instruments TMS320C2000 DSP family"                                },
    { EM_TI_C5500     , "Texas Instruments TMS320C55x DSP family"                                 },
    { EM_res143       , "Reserved"                                                                },
    { EM_res144       , "Reserved"                                                                },
    { EM_res145       , "Reserved"                                                                },
    { EM_res146       , "Reserved"                                                                },
    { EM_res147       , "Reserved"                                                                },
    { EM_res148       , "Reserved"                                                                },
    { EM_res149       , "Reserved"                                                                },
    { EM_res150       , "Reserved"                                                                },
    { EM_res151       , "Reserved"                                                                },
    { EM_res152       , "Reserved"                                                                },
    { EM_res153       , "Reserved"                                                                },
    { EM_res154       , "Reserved"                                                                },
    { EM_res155       , "Reserved"                                                                },
    { EM_res156       , "Reserved"                                                                },
    { EM_res157       , "Reserved"                                                                },
    { EM_res158       , "Reserved"                                                                },
    { EM_res159       , "Reserved"                                                                },
    { EM_MMDSP_PLUS   , "STMicroelectronics 64bit VLIW Data Signal Processor"                     },
    { EM_CYPRESS_M8C  , "Cypress M8C microprocessor"                                              },
    { EM_R32C         , "Renesas R32C series microprocessors"                                     },
    { EM_TRIMEDIA     , "NXP Semiconductors TriMedia architecture family"                         },
    { EM_QDSP6        , "QUALCOMM DSP6 Processor"                                                 },
    { EM_8051         , "Intel 8051 and variants"                                                 },
    { EM_STXP7X       , "STMicroelectronics STxP7x family"                                        },
    { EM_NDS32        , "Andes Technology compact code size embedded RISC processor family"       },
    { EM_ECOG1        , "Cyan Technology eCOG1X family"                                           },
    { EM_ECOG1X       , "Cyan Technology eCOG1X family"                                           },
    { EM_MAXQ30       , "Dallas Semiconductor MAXQ30 Core Micro-controllers"                      },
    { EM_XIMO16       , "New Japan Radio (NJR) 16-bit DSP Processor"                              },
    { EM_MANIK        , "M2000 Reconfigurable RISC Microprocessor"                                },
    { EM_CRAYNV2      , "Cray Inc. NV2 vector architecture"                                       },
    { EM_RX           , "Renesas RX family"                                                       },
    { EM_METAG        , "Imagination Technologies META processor architecture"                    },
    { EM_MCST_ELBRUS  , "MCST Elbrus general purpose hardware architecture"                       },
    { EM_ECOG16       , "Cyan Technology eCOG16 family"                                           },
    { EM_CR16         , "National Semiconductor CompactRISC 16-bit processor"                     },
    { EM_ETPU         , "Freescale Extended Time Processing Unit"                                 },
    { EM_SLE9X        , "Infineon Technologies SLE9X core"                                        },
    { EM_L1OM         , "Intel L1OM"                                                              },
    { EM_INTEL181     , "Reserved by Intel"                                                       },
    { EM_INTEL182     , "Reserved by Intel"                                                       },
    { EM_res183       , "Reserved by ARM"                                                         },
    { EM_res184       , "Reserved by ARM"                                                         },
    { EM_AVR32        , "Atmel Corporation 32-bit microprocessor family"                          },
    { EM_STM8         , "STMicroeletronics STM8 8-bit microcontroller"                            },
    { EM_TILE64       , "Tilera TILE64 multicore architecture family"                             },
    { EM_TILEPRO      , "Tilera TILEPro multicore architecture family"                            },
    { EM_MICROBLAZE   , "Xilinx MicroBlaze 32-bit RISC soft processor core"                       },
    { EM_CUDA         , "NVIDIA CUDA architecture "                                               },
};


//------------------------------------------------------------------------------
class dump
{
  public:
//------------------------------------------------------------------------------
    static void
    header( std::ostream& out, elfio& reader )
    {
        out << "ELF Header"     << std::endl << std::endl
            << "  Class:      " << str_class( reader.get_class() )         << std::endl
            << "  Encoding:   " << str_endian( reader.get_encoding() )     << std::endl
            << "  ELFVersion: " << str_version( reader.get_elf_version() ) << std::endl
            << "  Type:       " << str_type( reader.get_type() )           << std::endl
            << "  Machine:    " << str_machine( reader.get_machine() )     << std::endl
            << "  Version:    " << str_version( reader.get_version() )     << std::endl
            << "  Entry:      " << "0x" << std::hex << reader.get_entry()  << std::endl
            << "  Flags:      " << "0x" << std::hex << reader.get_flags()  << std::endl;
    }

  private:
    template< typename T, typename K >
    std::string
    static
    find_value_in_table( const T& table, const K& key )
    {
        std::string res = "UNKNOWN";
        for ( unsigned int i = 0; i < sizeof( table )/sizeof( table[0] ); ++i ) {
            if ( table[i].key == key ) {
                res = table[i].str;
                break;
            }
        }

        return res;
    }


    template< typename T, typename K >
    static
    std::string
    format_assoc( const T& table, const K& key )
    {
        std::string str = find_value_in_table( table, key );
        std::ostringstream oss;
        oss << str << " (0x" << std::hex << key << ")";

        return oss.str();
    }


#define STR_FUNC_TABLE( name )                              \
    static                                                  \
    std::string                                             \
    str_##name( const char key )                         \
    {                                                       \
        return format_assoc( name##_table, (int)key );   \
    }

    STR_FUNC_TABLE( class );
    STR_FUNC_TABLE( endian );
    STR_FUNC_TABLE( version );
    STR_FUNC_TABLE( type );
    STR_FUNC_TABLE( machine );

#undef STR_FUNC_TABLE
};
    

}; // namespace ELFIO

#endif // ELFIO_DUMP_HPP
