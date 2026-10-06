// Miles AIL calls: XMIDI driver DLL (AILXMI) and AIL16 timer services.
#ifndef AIL_H
#define AIL_H

typedef unsigned HSEQUENCE;

#ifdef __cplusplus
extern "C" {
#endif

void AIL_startup(void);
void AIL_shutdown(char *signoff_msg);
void pascal XMI_SERVE_DRIVER(void);
void *XMI_describe_driver(void);
int XMI_detect_device(unsigned io_addr, unsigned irq, unsigned dma, unsigned drq);
void XMI_init_driver(unsigned io_addr, unsigned irq, unsigned dma, unsigned drq);
void XMI_shutdown_driver(char *signoff_msg);
int XMI_state_table_size(void);
unsigned XMI_default_timbre_cache_size(void);
void XMI_define_timbre_cache(void *cache_addr, unsigned cache_size);
void XMI_install_timbre(int bank, int patch, void *src_addr);
HSEQUENCE XMI_register_sequence(void *form_xmid, unsigned sequence_num, void *state_table, void *controller_table);
void XMI_release_sequence_handle(HSEQUENCE sequence);
unsigned XMI_timbre_request(HSEQUENCE sequence);
void XMI_start_sequence(HSEQUENCE sequence);
void XMI_stop_sequence(HSEQUENCE sequence);
void XMI_resume_sequence(HSEQUENCE sequence);
unsigned XMI_sequence_status(HSEQUENCE sequence);
void XMI_set_relative_volume(HSEQUENCE sequence, unsigned percent, unsigned milliseconds);
void XMI_set_relative_tempo(HSEQUENCE sequence, unsigned percent, unsigned milliseconds);
int XMI_measure_count(HSEQUENCE sequence);
int XMI_branch_index(HSEQUENCE sequence, unsigned marker);

#ifdef __cplusplus
}
#endif

#endif
