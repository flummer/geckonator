/* USARTn_CTRL */
static inline void
usartn_(config, uint32_t v)               { USARTn->CTRL = v; }
static inline void
usartn_(autotx_disable, void)             { USARTn->CTRL &= ~USART_CTRL_AUTOTX; }
static inline void
usartn_(autotx_enable, void)              { USARTn->CTRL |= USART_CTRL_AUTOTX; }

/* USARTn_FRAME */
static inline void
usartn_(frame_8n1, void)
{
	USARTn->FRAME = USART_FRAME_STOPBITS_ONE
	              | USART_FRAME_PARITY_NONE
		      | USART_FRAME_DATABITS_EIGHT;
}
static inline void
usartn_(frame_bits, unsigned int n)
{
	USARTn->FRAME = n - 3;
}

/* USARTn_TRIGCTRL */
static inline void
usartn_(trigger_config, uint32_t v)       { USARTn->TRIGCTRL = v; }

/* USARTn_CMD */
static inline void
usartn_(rx_clear, void)                   { USARTn->CMD = USART_CMD_CLEARRX; }
static inline void
usartn_(tx_clear, void)                   { USARTn->CMD = USART_CMD_CLEARTX; }
static inline void
usartn_(tx_tristate_disable, void)        { USARTn->CMD = USART_CMD_TXTRIDIS; }
static inline void
usartn_(tx_tristate_enable, void)         { USARTn->CMD = USART_CMD_TXTRIEN; }
static inline void
usartn_(rx_block_disable, void)           { USARTn->CMD = USART_CMD_RXBLOCKDIS; }
static inline void
usartn_(rx_block_enable, void)            { USARTn->CMD = USART_CMD_RXBLOCKEN; }
static inline void
usartn_(master_disable, void)             { USARTn->CMD = USART_CMD_MASTERDIS; }
static inline void
usartn_(master_enable, void)              { USARTn->CMD = USART_CMD_MASTEREN; }
static inline void
usartn_(tx_disable, void)                 { USARTn->CMD = USART_CMD_TXDIS; }
static inline void
usartn_(tx_enable, void)                  { USARTn->CMD = USART_CMD_TXEN; }
static inline void
usartn_(rx_disable, void)                 { USARTn->CMD = USART_CMD_RXDIS; }
static inline void
usartn_(rx_enable, void)                  { USARTn->CMD = USART_CMD_RXEN; }
static inline void
usartn_(rx_disable_and_clear, void)       { USARTn->CMD = USART_CMD_RXDIS | USART_CMD_CLEARRX; }
static inline void
usartn_(rxtx_disable, void)               { USARTn->CMD = USART_CMD_TXDIS | USART_CMD_RXDIS; }
static inline void
usartn_(rxtx_enable, void)                { USARTn->CMD = USART_CMD_TXEN | USART_CMD_RXEN; }

/* USARTn_STATUS */
static inline uint32_t
usartn_(rx_full_right, void)              { return USARTn->STATUS & USART_STATUS_RXFULLRIGHT; }
static inline uint32_t
usartn_(rx_right, void)                   { return USARTn->STATUS & USART_STATUS_RXDATAVRIGHT; }
static inline uint32_t
usartn_(tx_right, void)                   { return USARTn->STATUS & USART_STATUS_TXBSRIGHT; }
static inline uint32_t
usartn_(tx_double_right, void)            { return USARTn->STATUS & USART_STATUS_TXBDRIGHT; }
static inline uint32_t
usartn_(rx_full, void)                    { return USARTn->STATUS & USART_STATUS_RXFULL; }
static inline uint32_t
usartn_(rx_valid, void)                   { return USARTn->STATUS & USART_STATUS_RXDATAV; }
static inline uint32_t
usartn_(tx_buffer_level, void)            { return USARTn->STATUS & USART_STATUS_TXBL; }
static inline uint32_t
usartn_(tx_complete, void)                { return USARTn->STATUS & USART_STATUS_TXC; }
static inline uint32_t
usartn_(tx_tristated, void)               { return USARTn->STATUS & USART_STATUS_TXTRI; }
static inline uint32_t
usartn_(rx_blocked, void)                 { return USARTn->STATUS & USART_STATUS_RXBLOCK; }
static inline uint32_t
usartn_(master_enabled, void)             { return USARTn->STATUS & USART_STATUS_MASTER; }
static inline uint32_t
usartn_(tx_enabled, void)                 { return USARTn->STATUS & USART_STATUS_TXENS; }
static inline uint32_t
usartn_(rx_enabled, void)                 { return USARTn->STATUS & USART_STATUS_RXENS; }

/* USARTn_CLKDIV */
static inline void
usartn_(clock_div, uint32_t v)            { USARTn->CLKDIV = v; }

/* USARTn_RXDATAX */

/* USARTn_RXDATA */
static inline uint32_t
usartn_(rxdata, void)                     { return USARTn->RXDATA; }

/* USARTn_RXDOUBLEX */

/* USARTn_RXDOUBLE */
static inline uint32_t
usartn_(rxdouble, void)                   { return USARTn->RXDOUBLE; }

/* USARTn_RXDATAXP */

/* USARTn_RXDOUBLXP */

/* USARTn_TXDATAX */
static inline void
usartn_(txdatax, uint32_t v)              { USARTn->TXDATAX = v; }

/* USARTn_TXDATA */
static inline void
usartn_(txdata, uint32_t v)               { USARTn->TXDATA = v; }

/* USARTn_TXDOUBLEX */
static inline void
usartn_(txdoublex, uint32_t v)            { USARTn->TXDOUBLEX = v; }

/* USARTn_TXDOUBLE */
static inline void
usartn_(txdouble, uint32_t v)             { USARTn->TXDOUBLE = v; }

/* USARTn_IF */
static inline uint32_t
usartn_(flags, void)                      { return USARTn->IF; }
static inline uint32_t
usartn_(flag_collision, uint32_t v)       { return v & USART_IF_CCF; }
static inline uint32_t
usartn_(flag_slave, uint32_t v)           { return v & USART_IF_SSM; }
static inline uint32_t
usartn_(flag_mp_address, uint32_t v)      { return v & USART_IF_MPAF; }
static inline uint32_t
usartn_(flag_framing_error, uint32_t v)   { return v & USART_IF_FERR; }
static inline uint32_t
usartn_(flag_parity_error, uint32_t v)    { return v & USART_IF_PERR; }
static inline uint32_t
usartn_(flag_tx_underflow, uint32_t v)    { return v & USART_IF_TXUF; }
static inline uint32_t
usartn_(flag_tx_overflow, uint32_t v)     { return v & USART_IF_TXOF; }
static inline uint32_t
usartn_(flag_rx_underflow, uint32_t v)    { return v & USART_IF_RXUF; }
static inline uint32_t
usartn_(flag_rx_overflow, uint32_t v)     { return v & USART_IF_RXOF; }
static inline uint32_t
usartn_(flag_rx_full, uint32_t v)         { return v & USART_IF_RXFULL; }
static inline uint32_t
usartn_(flag_rx_valid, uint32_t v)        { return v & USART_IF_RXDATAV; }
static inline uint32_t
usartn_(flag_tx_buffer_level, uint32_t v) { return v & USART_IF_TXBL; }
static inline uint32_t
usartn_(flag_tx_complete, uint32_t v)     { return v & USART_IF_TXC; }

/* USARTn_IFS */
static inline void
usartn_(flags_set, uint32_t v)            { USARTn->IFS = v; }
static inline void
usartn_(flag_collision_set, void)         { USARTn->IFS = USART_IFS_CCF; }
static inline void
usartn_(flag_slave_set, void)             { USARTn->IFS = USART_IFS_SSM; }
static inline void
usartn_(flag_mp_address_set, void)        { USARTn->IFS = USART_IFS_MPAF; }
static inline void
usartn_(flag_framing_error_set, void)     { USARTn->IFS = USART_IFS_FERR; }
static inline void
usartn_(flag_parity_error_set, void)      { USARTn->IFS = USART_IFS_PERR; }
static inline void
usartn_(flag_tx_underflow_set, void)      { USARTn->IFS = USART_IFS_TXUF; }
static inline void
usartn_(flag_tx_overflow_set, void)       { USARTn->IFS = USART_IFS_TXOF; }
static inline void
usartn_(flag_rx_underflow_set, void)      { USARTn->IFS = USART_IFS_RXUF; }
static inline void
usartn_(flag_rx_overflow_set, void)       { USARTn->IFS = USART_IFS_RXOF; }
static inline void
usartn_(flag_rx_full_set, void)           { USARTn->IFS = USART_IFS_RXFULL; }
static inline void
usartn_(flag_tx_complete_set, void)       { USARTn->IFS = USART_IFS_TXC; }

/* USARTn_IFC */
static inline void
usartn_(flags_clear, uint32_t v)
{
	USARTn->IFC = v &
		( USART_IFC_CCF
		| USART_IFC_SSM
		| USART_IFC_MPAF
		| USART_IFC_FERR
		| USART_IFC_PERR
		| USART_IFC_TXUF
		| USART_IFC_TXOF
		| USART_IFC_RXUF
		| USART_IFC_RXOF
		| USART_IFC_RXFULL
		| USART_IFC_TXC);
}
static inline void
usartn_(flags_clear_all, void)
{
	USARTn->IFC =
		( USART_IFC_CCF
		| USART_IFC_SSM
		| USART_IFC_MPAF
		| USART_IFC_FERR
		| USART_IFC_PERR
		| USART_IFC_TXUF
		| USART_IFC_TXOF
		| USART_IFC_RXUF
		| USART_IFC_RXOF
		| USART_IFC_RXFULL
		| USART_IFC_TXC);
}
static inline void
usartn_(flag_collision_clear, void)       { USARTn->IFC = USART_IFC_CCF; }
static inline void
usartn_(flag_slave_clear, void)           { USARTn->IFC = USART_IFC_SSM; }
static inline void
usartn_(flag_mp_address_clear, void)      { USARTn->IFC = USART_IFC_MPAF; }
static inline void
usartn_(flag_framing_error_clear, void)   { USARTn->IFC = USART_IFC_FERR; }
static inline void
usartn_(flag_parity_error_clear, void)    { USARTn->IFC = USART_IFC_PERR; }
static inline void
usartn_(flag_tx_underflow_clear, void)    { USARTn->IFC = USART_IFC_TXUF; }
static inline void
usartn_(flag_tx_overflow_clear, void)     { USARTn->IFC = USART_IFC_TXOF; }
static inline void
usartn_(flag_rx_underflow_clear, void)    { USARTn->IFC = USART_IFC_RXUF; }
static inline void
usartn_(flag_rx_overflow_clear, void)     { USARTn->IFC = USART_IFC_RXOF; }
static inline void
usartn_(flag_rx_full_clear, void)         { USARTn->IFC = USART_IFC_RXFULL; }
static inline void
usartn_(flag_tx_complete_clear, void)     { USARTn->IFC = USART_IFC_TXC; }

/* USUARTn_IEN */
static inline void
usartn_(flag_collision_disable, void)       { USARTn->IEN &= ~USART_IEN_CCF; }
static inline void
usartn_(flag_collision_enable, void)        { USARTn->IEN |= USART_IEN_CCF; }
static inline void
usartn_(flag_slave_disable, void)           { USARTn->IEN &= ~USART_IEN_SSM; }
static inline void
usartn_(flag_slave_enable, void)            { USARTn->IEN |= USART_IEN_SSM; }
static inline void
usartn_(flag_mp_address_disable, void)      { USARTn->IEN &= ~USART_IEN_MPAF; }
static inline void
usartn_(flag_mp_address_enable, void)       { USARTn->IEN |= USART_IEN_MPAF; }
static inline void
usartn_(flag_framing_error_disable, void)   { USARTn->IEN &= ~USART_IEN_FERR; }
static inline void
usartn_(flag_framing_error_enable, void)    { USARTn->IEN |= USART_IEN_FERR; }
static inline void
usartn_(flag_parity_error_disable, void)    { USARTn->IEN &= ~USART_IEN_PERR; }
static inline void
usartn_(flag_parity_error_enable, void)     { USARTn->IEN |= USART_IEN_PERR; }
static inline void
usartn_(flag_tx_underflow_disable, void)    { USARTn->IEN &= ~USART_IEN_TXUF; }
static inline void
usartn_(flag_tx_underflow_enable, void)     { USARTn->IEN |= USART_IEN_TXUF; }
static inline void
usartn_(flag_tx_overflow_disable, void)     { USARTn->IEN &= ~USART_IEN_TXOF; }
static inline void
usartn_(flag_tx_overflow_enable, void)      { USARTn->IEN |= USART_IEN_TXOF; }
static inline void
usartn_(flag_rx_underflow_disable, void)    { USARTn->IEN &= ~USART_IEN_RXUF; }
static inline void
usartn_(flag_rx_underflow_enable, void)     { USARTn->IEN |= USART_IEN_RXUF; }
static inline void
usartn_(flag_rx_overflow_disable, void)     { USARTn->IEN &= ~USART_IEN_RXOF; }
static inline void
usartn_(flag_rx_overflow_enable, void)      { USARTn->IEN |= USART_IEN_RXOF; }
static inline void
usartn_(flag_rx_full_disable, void)         { USARTn->IEN &= ~USART_IEN_RXFULL; }
static inline void
usartn_(flag_rx_full_enable, void)          { USARTn->IEN |= USART_IEN_RXFULL; }
static inline void
usartn_(flag_rx_valid_disable, void)        { USARTn->IEN &= ~USART_IEN_RXDATAV; }
static inline void
usartn_(flag_rx_valid_enable, void)         { USARTn->IEN |= USART_IEN_RXDATAV; }
static inline void
usartn_(flag_tx_buffer_level_disable, void) { USARTn->IEN &= ~USART_IEN_TXBL; }
static inline void
usartn_(flag_tx_buffer_level_enable, void)  { USARTn->IEN |= USART_IEN_TXBL; }
static inline void
usartn_(flag_tx_complete_disable, void)     { USARTn->IEN &= ~USART_IEN_TXC; }
static inline void
usartn_(flag_tx_complete_enable, void)      { USARTn->IEN |= USART_IEN_TXC; }

/* USARTn_IRCTRL */

/* USARTn_ROUTE */
static inline void
usartn_(pins, uint32_t v)                   { USARTn->ROUTE = v; }

/* USARTn_INPUT */

/* USARTn_I2SCTRL */
static inline void
usartn_(i2s_disable, void)                  { USARTn->I2SCTRL = 0; }
static inline void
usartn_(i2s_16bit_stereo, void)
{
	USARTn->I2SCTRL = USART_I2SCTRL_FORMAT_W16D16
	                | USART_I2SCTRL_DELAY
			| USART_I2SCTRL_EN;
}
static inline void
usartn_(i2s_16bit_stereo_32bit_words, void)
{
	USARTn->I2SCTRL = USART_I2SCTRL_FORMAT_W32D16
	                | USART_I2SCTRL_DELAY
			| USART_I2SCTRL_EN;
}
