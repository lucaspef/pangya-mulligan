#pragma once

// Registros inteiros que cabem no arquivo, limitados ao nRecords do cabecalho.
inline int IffRecordCount(int nRecords, int fileSize, int headerSize, int recordSize)
{
	const int available = fileSize > headerSize ? (fileSize - headerSize) / recordSize : 0;
	return nRecords < available ? nRecords : available;
}
