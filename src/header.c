#include "guppirawc99/header.h"

static const uint64_t KEY_UINT64_BLOCSIZE  = GUPPI_RAW_KEY_UINT64_ID_LE('B','L','O','C','S','I','Z','E');
static const uint64_t KEY_UINT64_NANTS     = GUPPI_RAW_KEY_UINT64_ID_LE('N','A','N','T','S',' ',' ',' ');
static const uint64_t KEY_UINT64_NBEAMS    = GUPPI_RAW_KEY_UINT64_ID_LE('N','B','E','A','M','S',' ',' ');
static const uint64_t KEY_UINT64_OBSNCHAN  = GUPPI_RAW_KEY_UINT64_ID_LE('O','B','S','N','C','H','A','N');
static const uint64_t KEY_UINT64_NPOL      = GUPPI_RAW_KEY_UINT64_ID_LE('N','P','O','L',' ',' ',' ',' ');
static const uint64_t KEY_UINT64_NBITS     = GUPPI_RAW_KEY_UINT64_ID_LE('N','B','I','T','S',' ',' ',' ');
static const uint64_t KEY_UINT64_DIRECTIO  = GUPPI_RAW_KEY_UINT64_ID_LE('D','I','R','E','C','T','I','O');

static const uint64_t KEY_UINT64_END  = GUPPI_RAW_KEY_UINT64_ID_LE('E','N','D',' ',' ',' ',' ',' ');
static const uint64_t _UINT64_BLANK   = GUPPI_RAW_KEY_UINT64_ID_LE(' ',' ',' ',' ',' ',' ',' ',' ');

void guppiraw_header_parse_entry(const char* entry, guppiraw_metadata_t* metadata) {
  if(((uint64_t*)entry)[0] == KEY_UINT64_BLOCSIZE)
    hgetu8(entry, "BLOCSIZE", &metadata->datashape.block_size);
  else if(((uint64_t*)entry)[0] == KEY_UINT64_NANTS)
    hgetu4(entry, "NANTS", &metadata->datashape.n_ant);
  else if(((uint64_t*)entry)[0] == KEY_UINT64_NBEAMS)
    hgetu4(entry, "NBEAMS", &metadata->datashape.n_beam);
  else if(((uint64_t*)entry)[0] == KEY_UINT64_OBSNCHAN)
    hgetu4(entry, "OBSNCHAN", &metadata->datashape.n_obschan);
  else if(((uint64_t*)entry)[0] == KEY_UINT64_NPOL)
    hgetu4(entry, "NPOL", &metadata->datashape.n_pol);
  else if(((uint64_t*)entry)[0] == KEY_UINT64_NBITS)
    hgetu4(entry, "NBITS", &metadata->datashape.n_bit);
  else if(((uint64_t*)entry)[0] == KEY_UINT64_DIRECTIO)
    hgeti4(entry, "DIRECTIO", &metadata->directio);

  if(metadata->user_callback != 0) {
    metadata->user_callback(entry, metadata->user_data);
  }
}

char guppiraw_header_entry_is_END(const uint64_t* entry_uint64) {
  return entry_uint64[0] == KEY_UINT64_END &&
    entry_uint64[1] == _UINT64_BLANK &&
    entry_uint64[2] == _UINT64_BLANK &&
    entry_uint64[3] == _UINT64_BLANK &&
    entry_uint64[4] == _UINT64_BLANK &&
    entry_uint64[5] == _UINT64_BLANK &&
    entry_uint64[6] == _UINT64_BLANK &&
    entry_uint64[7] == _UINT64_BLANK &&
    entry_uint64[8] == _UINT64_BLANK &&
    entry_uint64[9] == _UINT64_BLANK;
}

void guppiraw_header_parse(guppiraw_header_t* header, char* header_string, int64_t header_string_length) {
  if(header->head != NULL) {
    free(header->head);
  }
  header->n_entries = 0;
  header->head = malloc(sizeof(guppiraw_header_llnode_t));
	guppiraw_header_llnode_t* head = header->head;
	
  while(
    !guppiraw_header_entry_is_END((uint64_t*)header_string) && 
    (header_string_length >= 80 || header_string_length < 0)
  ) {
    guppiraw_header_parse_entry(header_string, &header->metadata);

		if(header->n_entries > 0) {
			head->next = malloc(sizeof(guppiraw_header_llnode_t));
			head = head->next;
		}
		memcpy(head->keyvalue, header_string, 80);
		head->keyvalue[80] = '\0';
		header->n_entries++;

    header_string += 80;
    header_string_length -= 80;
  }
	head->next = NULL;
}

void guppiraw_header_string_parse_metadata(guppiraw_metadata_t* metadata, char* header_string, int64_t header_string_length) {
  while(
    !guppiraw_header_entry_is_END((uint64_t*)header_string) && 
    (header_string_length >= 80 || header_string_length < 0)
  ) {
    guppiraw_header_parse_entry(header_string, metadata);
    header_string += 80;
    header_string_length -= 80;
  }
}

static char _GUPPI_RAW_FTISHEADER_VALUEBUF_STR[] =
"                                                                                "
GUPPI_RAW_HEADER_END_STR;  

int _guppiraw_header_put_entry(guppiraw_header_llnode_t* head, const char* keyvalue) {
  while(1) {
    if(strncmp(head->keyvalue, keyvalue, 8) == 0) {
      memcpy(head->keyvalue, keyvalue, 80);
			head->keyvalue[80] = '\0';
      return 0;
    }

    if(head->next == NULL) {
      break;
    }
    head = head->next;
  }
  head->next = malloc(sizeof(guppiraw_header_llnode_t));
  memcpy(head->next->keyvalue, _GUPPI_RAW_FTISHEADER_VALUEBUF_STR, 80);
	head->next->keyvalue[80] = '\0';
  head->next->next = NULL;
  return 1;
}

int _guppiraw_header_put_string(guppiraw_header_llnode_t* head, const char* key, const char* value) {
  memset(_GUPPI_RAW_FTISHEADER_VALUEBUF_STR, ' ', 80);
  hputs(_GUPPI_RAW_FTISHEADER_VALUEBUF_STR, key, value);
  return _guppiraw_header_put_entry(head, _GUPPI_RAW_FTISHEADER_VALUEBUF_STR);
}

int _guppiraw_header_put_double(guppiraw_header_llnode_t* head, const char* key, const double value) {
  memset(_GUPPI_RAW_FTISHEADER_VALUEBUF_STR, ' ', 80);
  hputr8(_GUPPI_RAW_FTISHEADER_VALUEBUF_STR, key, value);
  return _guppiraw_header_put_entry(head, _GUPPI_RAW_FTISHEADER_VALUEBUF_STR);
}

int _guppiraw_header_put_integer(guppiraw_header_llnode_t* head, const char* key, const int64_t value) {
  memset(_GUPPI_RAW_FTISHEADER_VALUEBUF_STR, ' ', 80);
  hputi8(_GUPPI_RAW_FTISHEADER_VALUEBUF_STR, key, value);
  return _guppiraw_header_put_entry(head, _GUPPI_RAW_FTISHEADER_VALUEBUF_STR);
}

static inline void _guppiraw_header_ensure_initialised(guppiraw_header_t* header, const char* key) {
  if(header->head == NULL || header->n_entries == 0) {
    header->head = malloc(sizeof(guppiraw_header_llnode_t));
    memset(header->head->keyvalue, ' ', 80);
    memcpy(header->head->keyvalue, key, strlen(key));
    header->head->next = NULL;
    header->n_entries = 1;
  }
}

int guppiraw_header_put_string(guppiraw_header_t* header, const char* key, const char* value) {
  _guppiraw_header_ensure_initialised(header, key);
  header->n_entries += _guppiraw_header_put_string(header->head, key, value);
  return 0;
}
int guppiraw_header_put_double(guppiraw_header_t* header, const char* key, const double value) {
  _guppiraw_header_ensure_initialised(header, key);
  header->n_entries += _guppiraw_header_put_double(header->head, key, value);
  return 0;
}
int guppiraw_header_put_integer(guppiraw_header_t* header, const char* key, const int64_t value) {
  _guppiraw_header_ensure_initialised(header, key);
  header->n_entries += _guppiraw_header_put_integer(header->head, key, value);
  return 0;
}
int guppiraw_header_put_metadata(guppiraw_header_t* header) {
	guppiraw_header_put_integer(header, "NBITS", header->metadata.datashape.n_bit);
	guppiraw_header_put_integer(header, "NPOL", header->metadata.datashape.n_pol);
	guppiraw_header_put_integer(header, "NANTS", header->metadata.datashape.n_ant);
  header->metadata.datashape.n_aspect = header->metadata.datashape.n_ant;
  if(header->metadata.datashape.n_beam > 0) {
	  guppiraw_header_put_integer(header, "NBEAMS", header->metadata.datashape.n_beam);
    header->metadata.datashape.n_aspect = header->metadata.datashape.n_beam;
  }
  header->metadata.datashape.block_size = (
    header->metadata.datashape.n_aspect*header->metadata.datashape.n_aspectchan
    *header->metadata.datashape.n_time*header->metadata.datashape.n_pol
    *2*header->metadata.datashape.n_bit)/8
  ;
  
	guppiraw_header_put_integer(header, "OBSNCHAN", header->metadata.datashape.n_aspect*header->metadata.datashape.n_aspectchan);
	guppiraw_header_put_integer(header, "BLOCSIZE", header->metadata.datashape.block_size);
	guppiraw_header_put_integer(header, "DIRECTIO", header->metadata.directio);
  return 0;
}


void _guppiraw_header_free(guppiraw_header_llnode_t* head) {
  if(head->next != NULL) {
    _guppiraw_header_free(head->next);
  }
  free(head->next);
}

void guppiraw_header_free(guppiraw_header_t* header) {
  _guppiraw_header_free(header->head);
  free(header->head);
  header->n_entries = 0;
}

const char _guppiraw_directio_padding_buffer[513] = 
"********************************************************************************************************************************"
"********************************************************************************************************************************"
"********************************************************************************************************************************"
"********************************************************************************************************************************";

char* guppiraw_header_malloc_string(const guppiraw_header_t* header) {
  const char directio = header->metadata.directio;
  const int n_entries = header->n_entries;
  const size_t header_entries_len = (n_entries + 1) * 80;
  guppiraw_header_llnode_t* header_entry = header->head;
  char* header_string;
  if(directio) {
    const size_t header_entries_len_aligned = guppiraw_directio_align(header_entries_len);
    header_string = memalign(512, header_entries_len_aligned);
    memcpy(
      header_string + header_entries_len,
      _guppiraw_directio_padding_buffer,
      header_entries_len_aligned - header_entries_len
    );
  }
  else {
    header_string = malloc(header_entries_len);
  }

  for(int i = 0; i < n_entries; i++) {
    memcpy(header_string + i*80, header_entry->keyvalue, 80);
    header_entry = header_entry->next;
  }
  memcpy(header_string + n_entries*80, GUPPI_RAW_HEADER_END_STR, 80);
  return header_string;
}
