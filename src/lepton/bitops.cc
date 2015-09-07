/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
This file contains special classes for bitwise
reading and writing of arrays
*/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <algorithm>
#include <assert.h>
#include "bitops.hh"

#define BUFFER_SIZE 1024 * 1024


/* -----------------------------------------------
	constructor for abitreader class
	----------------------------------------------- */	

abitreader::abitreader( unsigned char* array, int size )
{
	cbyte = 0;	
	cbit = 8;
	eof = false;
	
	data = array;
	lbyte = size;	
}

/* -----------------------------------------------
	destructor for abitreader class
	----------------------------------------------- */	

abitreader::~abitreader( void )
{
}

/* -----------------------------------------------
	reads n bits from abitreader
	----------------------------------------------- */	

unsigned int abitreader::read( int nbits )
{
	unsigned int retval = 0;
	
	// safety check for eof
	if (eof) {
        return 0;
    }
	
	while ( nbits >= cbit ) {
		nbits -= cbit;
		retval |= ( RBITS( data[cbyte], cbit ) << nbits );		
		cbit = 8;
		if ( ++cbyte >= lbyte ) {
			eof = true;
			return retval;
		}
	}
	
	if ( nbits > 0 ) {		
		retval |= ( MBITS( data[cbyte], cbit, (cbit-nbits) ) );
		cbit -= nbits;		
	}
	
	return retval;
}

/* -----------------------------------------------
	to skip padding from current byte
	----------------------------------------------- */

unsigned char abitreader::unpad( unsigned char fillbit )
{
	if ( ( cbit == 8 ) || eof ) return fillbit;
	else {
		fillbit = read( 1 );
		while ( cbit != 8 ) read( 1 );
	}
	
	return fillbit;
}

/* -----------------------------------------------
	get current position in array
	----------------------------------------------- */	

int abitreader::getpos( void )
{
	return cbyte;
}


/* -----------------------------------------------
	constructor for abitwriter class
	----------------------------------------------- */	

abitwriter::abitwriter( int size )
{
	fillbit = 1;
	adds    = 65536;
    cbyte2   = 0;
    cbit2    = 64;
    buf = 0;
	
	error = false;
	fmem  = true;
	
	dsize = ( size > 0 ) ? size : adds;
    data2 = ( unsigned char* ) calloc ( dsize , 1);
	if ( data2 == NULL ) {
		error = true;
		return;
	}
	
	// for ( int i = 0; i < dsize; i++ ) data[i] = 0;
}

/* -----------------------------------------------
	destructor for abitwriter class
	----------------------------------------------- */	

abitwriter::~abitwriter( void )
{
	// free memory if pointer was not given out
    if ( fmem )	free( data2 );
}




/* -----------------------------------------------
	constructor for abytewriter class
	----------------------------------------------- */	

abytewriter::abytewriter( int size )
{
	adds  = 65536;
	cbyte = 0;
	
	error = false;
	fmem  = true;
	
	dsize = ( size > 0 ) ? size : adds;
	data = (unsigned char*) malloc( dsize );
	if ( data == NULL ) {
		error = true;
		return;
	}
}

/* -----------------------------------------------
	destructor for abytewriter class
	----------------------------------------------- */	

abytewriter::~abytewriter( void )
{
	// free data if pointer is not read
	if ( fmem )	free( data );
}

/* -----------------------------------------------
	writes 1 byte to abytewriter
	----------------------------------------------- */	

void abytewriter::write( unsigned char byte )
{
	// safety check for error
	if ( error ) return;
	
	// test if pointer beyond flush threshold
	if ( cbyte >= ( dsize - 2 ) ) {
		dsize += adds;
		data = (unsigned char*) realloc( data, dsize );
		if ( data == NULL ) {
			error = true;
			return;
		}
	}
	
	// write data
	data[ cbyte++ ] = byte;
}

/* -----------------------------------------------
	writes n byte to abytewriter
	----------------------------------------------- */
	
void abytewriter::write_n( unsigned char* byte, int n )
{
	// safety check for error
	if ( error ) return;
	
	// make sure that pointer doesn't get beyond flush threshold
	while ( ( cbyte + n ) >= ( dsize - 2 ) ) {
		dsize += adds;
		data = (unsigned char*) realloc( data, dsize );
		if ( data == NULL ) {
			error = true;
			return;
		}
	}
	
	// copy data from array
	while ( n-- > 0 )
		data[ cbyte++ ] = *(byte++);
}

/* -----------------------------------------------
	gets data array from abytewriter
	----------------------------------------------- */

unsigned char* abytewriter::getptr( void )
{
	// forbid freeing memory
	fmem = false;
	// realloc data
	data = (unsigned char*) realloc( data, cbyte );
	
	return data;
}

/* -----------------------------------------------
	peeks into data array from abytewriter
	----------------------------------------------- */
	
unsigned char* abytewriter::peekptr( void )
{
	return data;
}

/* -----------------------------------------------
	gets size of data array from abytewriter
	----------------------------------------------- */	

int abytewriter::getpos( void )
{
	return cbyte;
}

/* -----------------------------------------------
	reset without realloc
	----------------------------------------------- */	
	
void abytewriter::reset( void )
{
	// set position of current byte
	cbyte = 0;
}


/* -----------------------------------------------
	constructor for abytewriter class
	----------------------------------------------- */

abytereader::abytereader( unsigned char* array, int size )
{
	cbyte = 0;
	eof = false;
	
	data = array;
	lbyte = size;
	
	if ( ( data == NULL ) || ( lbyte == 0 ) )
		eof = true;
}

/* -----------------------------------------------
	destructor for abytewriter class
	----------------------------------------------- */

abytereader::~abytereader( void )
{
}

/* -----------------------------------------------
	reads 1 byte from abytereader
	----------------------------------------------- */

int abytereader::read( unsigned char* byte )
{
	if ( cbyte >= lbyte ) {
		cbyte = lbyte;
		eof = true;
		return 0;
	}
	else {
		*byte = data[ cbyte++ ];
		return 1;
	}
}

/* -----------------------------------------------
	reads n bytes from abytereader
	----------------------------------------------- */
	
int abytereader::read_n( unsigned char* byte, int n )
{
	int nl = lbyte - cbyte;
	int i;
	
	if ( nl < n ) {
		for ( i = 0; i < nl; i++ )
			byte[ i ] = data[ cbyte + i ];
		cbyte = lbyte;
		eof = true;
		return nl;
	}
	else {
		for ( i = 0; i < n; i++ )
			byte[ i ] = data[ cbyte + i ];
		cbyte += n;
		return n;
	}
}

/* -----------------------------------------------
	go to position in data
	----------------------------------------------- */
	
void abytereader::seek( int pos )
{
	if ( pos >= lbyte ) {
		cbyte = lbyte;
		eof = true;
	}
	else {
		cbyte = pos;
		eof = false;
	}
}

/* -----------------------------------------------
	gets size of current data
	----------------------------------------------- */
	
int abytereader::getsize( void )
{
	return lbyte;
}

/* -----------------------------------------------
	gets current position from abytereader
	----------------------------------------------- */	

int abytereader::getpos( void )
{
	return cbyte;
}

bounded_iostream::bounded_iostream(Sirikata::DecoderWriter *w,
                                   const std::function<void(Sirikata::DecoderWriter*, size_t)> &size_callback,
                                   const Sirikata::JpegAllocator<uint8_t> &alloc) 
    : parent(w), err(Sirikata::JpegError::nil()) {
    this->size_callback = size_callback;
    buffer_position = 0;
    byte_position = 0;
    set_bound(0);
}
void bounded_iostream::call_size_callback(size_t size) {
    size_callback(parent, size);
}
bool bounded_iostream::chkerr() {
    return err != Sirikata::JpegError::nil();
}

void bounded_iostream::set_bound(size_t bound) {
    flush();
    byte_bound = bound;
}
void bounded_iostream::flush() {
    if (buffer_position) {
        write_no_buffer(buffer, buffer_position);
        buffer_position = 0;
    }
}
void bounded_iostream::close() {
    flush();
    parent->Close();
}

unsigned int bounded_iostream::write_no_buffer(const void *from, size_t bytes_to_write) {
    //return iostream::write(from,tpsize,dtsize);
    std::pair<unsigned int, Sirikata::JpegError> retval;
    if (byte_bound != 0 && byte_position + bytes_to_write > byte_bound) {
        bytes_to_write = byte_bound - byte_position;
        byte_position += bytes_to_write;
        retval = parent->Write(reinterpret_cast<const unsigned char*>(from), bytes_to_write);
        if (retval.first < bytes_to_write) {
            err = retval.second;
            return retval.first;
        }
        return bytes_to_write; // pretend we wrote it all
    }
    size_t total = bytes_to_write;
    retval = parent->Write(reinterpret_cast<const unsigned char*>(from), total);
    unsigned int written = retval.first;
    byte_position += written;
    if (written < total ) {
        err = retval.second;
        return written;
    }
    return bytes_to_write;
}

unsigned int bounded_iostream::getsize() {
    return byte_position;
}

bounded_iostream::~bounded_iostream(){
}

unsigned int ibytestream::getsize () {
    return bytes_read;
}

unsigned char ibytestream::get_last_read() {
    return last_read[1];
}

unsigned char ibytestream::get_penultimate_read() {
    return last_read[0];
}

ibytestream::ibytestream(Sirikata::DecoderReader *p, unsigned int byte_offset,
                         const Sirikata::JpegAllocator<uint8_t> &alloc) 
    : parent(p) {
    bytes_read = byte_offset;
}

unsigned int ibytestream::read(unsigned char*output, unsigned int size) {
    assert(size);
    if (size == 1) {
        return read_byte(output) ? 1 : 0;
    }
    int retval = IOUtil::ReadFull(parent, output, size);
    bytes_read += retval;
    static_assert(sizeof(last_read) == 2, "Last read must hold full jpeg huffman");
    if (retval >= 2) {
        memcpy(last_read, output + size - sizeof(last_read), sizeof(last_read));
    } else if (retval) {
        last_read[0] = last_read[1];
        last_read[1] = *output;
    }
    return retval;
}

bool ibytestream::read_byte(unsigned char *output) {
    unsigned int retval = parent->Read(output, 1).first;
    if (retval != 0) {
        last_read[0] = last_read[1];
        last_read[1] = *output;
        bytes_read += 1;
        return true;
    }
    return false;
}
