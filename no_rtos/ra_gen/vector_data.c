/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = spi_rxi_isr, /* SPI1 RXI (Receive buffer full) */
            [1] = spi_tei_isr, /* SPI1 TEI (Transmission complete event) */
            [2] = spi_eri_isr, /* SPI1 ERI (Error) */
            [3] = dmac_int_isr, /* DMAC1 INT (DMAC transfer end 1) */
            [4] = gpt_counter_overflow_isr, /* GPT0 COUNTER OVERFLOW (Overflow) */
            [5] = iic_master_rxi_isr, /* IIC2 RXI (Receive data full) */
            [6] = iic_master_txi_isr, /* IIC2 TXI (Transmit data empty) */
            [7] = iic_master_tei_isr, /* IIC2 TEI (Transmit end) */
            [8] = iic_master_eri_isr, /* IIC2 ERI (Transfer error) */
        };
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_IELS_ENUM(EVENT_SPI1_RXI), /* SPI1 RXI (Receive buffer full) */
            [1] = BSP_PRV_IELS_ENUM(EVENT_SPI1_TEI), /* SPI1 TEI (Transmission complete event) */
            [2] = BSP_PRV_IELS_ENUM(EVENT_SPI1_ERI), /* SPI1 ERI (Error) */
            [3] = BSP_PRV_IELS_ENUM(EVENT_DMAC1_INT), /* DMAC1 INT (DMAC transfer end 1) */
            [4] = BSP_PRV_IELS_ENUM(EVENT_GPT0_COUNTER_OVERFLOW), /* GPT0 COUNTER OVERFLOW (Overflow) */
            [5] = BSP_PRV_IELS_ENUM(EVENT_IIC2_RXI), /* IIC2 RXI (Receive data full) */
            [6] = BSP_PRV_IELS_ENUM(EVENT_IIC2_TXI), /* IIC2 TXI (Transmit data empty) */
            [7] = BSP_PRV_IELS_ENUM(EVENT_IIC2_TEI), /* IIC2 TEI (Transmit end) */
            [8] = BSP_PRV_IELS_ENUM(EVENT_IIC2_ERI), /* IIC2 ERI (Transfer error) */
        };
        #endif
