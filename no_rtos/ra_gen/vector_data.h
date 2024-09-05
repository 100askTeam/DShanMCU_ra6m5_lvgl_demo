/* generated vector header file - do not edit */
#ifndef VECTOR_DATA_H
#define VECTOR_DATA_H
#ifdef __cplusplus
        extern "C" {
        #endif
/* Number of interrupts allocated */
#ifndef VECTOR_DATA_IRQ_COUNT
#define VECTOR_DATA_IRQ_COUNT    (9)
#endif
/* ISR prototypes */
void spi_rxi_isr(void);
void spi_tei_isr(void);
void spi_eri_isr(void);
void dmac_int_isr(void);
void gpt_counter_overflow_isr(void);
void iic_master_rxi_isr(void);
void iic_master_txi_isr(void);
void iic_master_tei_isr(void);
void iic_master_eri_isr(void);

/* Vector table allocations */
#define VECTOR_NUMBER_SPI1_RXI ((IRQn_Type) 0) /* SPI1 RXI (Receive buffer full) */
#define SPI1_RXI_IRQn          ((IRQn_Type) 0) /* SPI1 RXI (Receive buffer full) */
#define VECTOR_NUMBER_SPI1_TEI ((IRQn_Type) 1) /* SPI1 TEI (Transmission complete event) */
#define SPI1_TEI_IRQn          ((IRQn_Type) 1) /* SPI1 TEI (Transmission complete event) */
#define VECTOR_NUMBER_SPI1_ERI ((IRQn_Type) 2) /* SPI1 ERI (Error) */
#define SPI1_ERI_IRQn          ((IRQn_Type) 2) /* SPI1 ERI (Error) */
#define VECTOR_NUMBER_DMAC1_INT ((IRQn_Type) 3) /* DMAC1 INT (DMAC1 transfer end) */
#define DMAC1_INT_IRQn          ((IRQn_Type) 3) /* DMAC1 INT (DMAC1 transfer end) */
#define VECTOR_NUMBER_GPT0_COUNTER_OVERFLOW ((IRQn_Type) 4) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define GPT0_COUNTER_OVERFLOW_IRQn          ((IRQn_Type) 4) /* GPT0 COUNTER OVERFLOW (Overflow) */
#define VECTOR_NUMBER_IIC2_RXI ((IRQn_Type) 5) /* IIC2 RXI (Receive data full) */
#define IIC2_RXI_IRQn          ((IRQn_Type) 5) /* IIC2 RXI (Receive data full) */
#define VECTOR_NUMBER_IIC2_TXI ((IRQn_Type) 6) /* IIC2 TXI (Transmit data empty) */
#define IIC2_TXI_IRQn          ((IRQn_Type) 6) /* IIC2 TXI (Transmit data empty) */
#define VECTOR_NUMBER_IIC2_TEI ((IRQn_Type) 7) /* IIC2 TEI (Transmit end) */
#define IIC2_TEI_IRQn          ((IRQn_Type) 7) /* IIC2 TEI (Transmit end) */
#define VECTOR_NUMBER_IIC2_ERI ((IRQn_Type) 8) /* IIC2 ERI (Transfer error) */
#define IIC2_ERI_IRQn          ((IRQn_Type) 8) /* IIC2 ERI (Transfer error) */
#ifdef __cplusplus
        }
        #endif
#endif /* VECTOR_DATA_H */
