// Main.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/02/01
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

// converts from V0 to V1

#include "ssi.h"
using namespace ssi;

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

void V0toVX (const ssi_char_t *filename, ssi_char_t *sample_type, bool list_input, bool keep_backup);
void V1toVX (const ssi_char_t *filename, bool list_input, bool keep_backup);
void VXtoWAV (const ssi_char_t *filename, bool list_input);
void WAVtoVX (const ssi_char_t *filename, bool list_input);
void VXRepair (const ssi_char_t *filename, bool list_input);
void V2Repair (const ssi_char_t *filename, bool list_input, ssi_time_t sr, ssi_type_t type, ssi_size_t dim);
void WAVRepair (const ssi_char_t *filename, bool list_input);
void VXtoCSV (const ssi_char_t *filename, bool list_input, bool csv_header);

int main (int argc, char **argv) {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	char info[1024];
	ssi_sprint (info, "\n%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

	//**** READ COMMAND LINE ****//

	CmdArgParser cmd;
	cmd.info (info);

	ssi_char_t *filename = 0;
	ssi_char_t *sample_type = 0;
	bool list_input, debug_to_file, keep_backup;
	bool csv_header;
	int dim = 0;
	int type = 0;
	double sr = 0;

	cmd.addMasterSwitch ("--V0toVX");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filename", &filename, "path of ssi file");
	cmd.addSCmdArg("type", &sample_type, "sample type (see list below)");

	cmd.addText ("\nOptions:");
	cmd.addBCmdOption ("-list", &list_input, false, "a text file with a list of ssi files is provided [false]");	
	cmd.addBCmdOption ("-backup", &keep_backup, false, "keep backup files [false]");	
	cmd.addBCmdOption ("-dbg2file", &debug_to_file, false, "debug to file [false]");	

	cmd.addText ("\nsample type names (mind case and spelling):\n");
	for (ssi_size_t i = 0; i < SSI_TYPE_NAME_SIZE; i++) {
		cmd.addText (SSI_TYPE_NAMES[i]);
	}

	cmd.addMasterSwitch ("--V1toVX");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filename", &filename, "path of ssi file");

	cmd.addText ("\nOptions:");
	cmd.addBCmdOption ("-list", &list_input, false, "a text file with a list of ssi files is provided [false]");	
	cmd.addBCmdOption ("-backup", &keep_backup, false, "keep backup files [false]");	
	cmd.addBCmdOption ("-dbg2file", &debug_to_file, false, "debug to file [false]");	

	cmd.addMasterSwitch ("--VXtoWAV");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filename", &filename, "path of ssi file");

	cmd.addText ("\nOptions:");
	cmd.addBCmdOption ("-list", &list_input, false, "a text file with a list of ssi files is provided [false]");		
	cmd.addBCmdOption ("-dbg2file", &debug_to_file, false, "debug to file [false]");	

	cmd.addMasterSwitch ("--WAVtoVX");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filename", &filename, "path of wav file");

	cmd.addText ("\nOptions:");
	cmd.addBCmdOption ("-list", &list_input, false, "a text file with a list of wav files is provided [false]");	
	cmd.addBCmdOption ("-dbg2file", &debug_to_file, false, "debug to file [false]");	

	cmd.addMasterSwitch ("--VXRepair");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filename", &filename, "path of corrupted ssi file");

	cmd.addText ("\nOptions:");
	cmd.addBCmdOption ("-list", &list_input, false, "a text file with a list of wav files is provided [false]");	
	cmd.addBCmdOption ("-dbg2file", &debug_to_file, false, "debug to file [false]");	

	cmd.addMasterSwitch ("--V2Repair");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filename", &filename, "path of corrupted ssi file (no extension)");
	cmd.addDCmdArg ("sr", &sr, "stream sample rate");
	cmd.addICmdArg ("type", &type, "stream type (SSI_UNDEF = 0, SSI_CHAR = 1, SSI_UCHAR = 2, SSI_SHORT = 3, SSI_USHORT = 4, SSI_INT = 5, SSI_UINT = 6, SSI_LONG = 7, SSI_ULONG = 8, SSI_FLOAT = 9,	SSI_DOUBLE = 10, SSI_LDOUBLE = 11");
	cmd.addICmdArg ("dim", &dim, "stream dimension");

	cmd.addText ("\nOptions:");
	cmd.addBCmdOption ("-list", &list_input, false, "a text file with a list of wav files is provided [false]");	
	cmd.addBCmdOption ("-dbg2file", &debug_to_file, false, "debug to file [false]");	

	cmd.addMasterSwitch ("--WAVRepair");

	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filename", &filename, "path of corrupted wav file");

	cmd.addText ("\nOptions:");
	cmd.addBCmdOption ("-list", &list_input, false, "a text file with a list of wav files is provided [false]");	
	cmd.addBCmdOption ("-dbg2file", &debug_to_file, false, "debug to file [false]");	

	cmd.addMasterSwitch ("--VXtoCSV");
	cmd.addText("\nArguments:");
	cmd.addSCmdArg("filename", &filename, "path of ssi file");

	cmd.addText ("\nOptions:");
	cmd.addBCmdOption ("-header", &csv_header, false, "add csv header [true]");
	cmd.addBCmdOption ("-list", &list_input, false, "a text file with a list of wav files is provided [false]");	
	cmd.addBCmdOption ("-dbg2file", &debug_to_file, false, "debug to file [false]");	

	if (cmd.read (argc, argv)) {		

		if (debug_to_file) {
			ssi_log_file_begin ("ssi_dbg.txt");
		}
		ssi_print ("%s", info);

		switch (cmd.master_switch) {

			case 1: {
				V0toVX (filename, sample_type, list_input, keep_backup);
				break;
			}
			case 2: {
				V1toVX (filename, list_input, keep_backup);
				break;
			}
			case 3: {
				VXtoWAV (filename, list_input);
				break;
			}
			case 4: {
				WAVtoVX (filename, list_input);
				break;
			}
			case 5: {
				VXRepair (filename, list_input);
				break;
			}
			case 6: {
				V2Repair (filename, list_input, sr, ssi_cast (ssi_type_t, type), ssi_cast (ssi_size_t, dim));
				break;
			}
			case 7: {
				WAVRepair (filename, list_input);
				break;
			}
			case 8: {
				VXtoCSV (filename, list_input, csv_header);
				break;
			}
			default: {
				ssi_err ("unkown master switch");
				break;
			}
		}
	}

	delete[] filename;
	delete[] sample_type;

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

void V0toVX (const ssi_char_t *filename, ssi_char_t *sample_type, bool list_input, bool keep_backup) {

	ssi_print ("convert=V0toVX\ntype=%s\nlist=%s\nbackup=%s\n\n", sample_type, list_input ? "true" : "false", keep_backup ? "true" : "false");

	ssi_type_t type = SSI_UNDEF;
	if (!ssi_name2type (sample_type, type)) {
		ssi_err ("unkown sample type '%s'", sample_type);
	}

	if (list_input) {
				
		StringList list;
		FileTools::ReadFilesFromFile (list, filename);
		for (ssi_size_t i = 0; i < list.size (); i++) {
			const ssi_char_t *filename = list.get (i);
			ssi_print ("converting '%s'..", filename);
			if (ssi_exists (filename)) {
				FileTools::ConvertStreamV0toV1Binary (filename, type, keep_backup);
				ssi_print ("ok\n");
			} else {
				ssi_print ("failed\n");
			}
		}
	} else {
		ssi_print ("converting '%s'..", filename);
		if (ssi_exists (filename)) {
			FileTools::ConvertStreamV0toV1Binary (filename, type, keep_backup);
			ssi_print ("ok\n");
		}  else {
			ssi_print ("failed\n");
		}
	}
}

void V1toVX (const ssi_char_t *filename, bool list_input, bool keep_backup) {

	ssi_print ("convert=V1toVX\nlist=%s\nbackup=%s\n\n", list_input ? "true" : "false", keep_backup ? "true" : "false");

	if (list_input) {
				
		StringList list;
		FileTools::ReadFilesFromFile (list, filename);
		for (ssi_size_t i = 0; i < list.size (); i++) {
			const ssi_char_t *filename = list.get (i);
			ssi_print ("converting '%s'..", filename);
			if (ssi_exists (filename)) {
				FileTools::ConvertStreamV1toV2Binary (filename, keep_backup);
				ssi_print ("ok\n");
			} else {
				ssi_print ("failed\n");
			}
		}
	} else {
		ssi_print ("converting '%s'..", filename);
		if (ssi_exists (filename)) {
			FileTools::ConvertStreamV1toV2Binary (filename, keep_backup);
			ssi_print ("ok\n");
		}  else {
			ssi_print ("failed\n");
		}
	}
}

void VXtoWAV (const ssi_char_t *filename, bool list_input) {

	ssi_print ("convert=VXtoWAV\nlist=%s\n\n", list_input ? "true" : "false");

	if (list_input) {
				
		StringList list;		
		FileTools::ReadFilesFromFile (list, filename);		
		for (ssi_size_t i = 0; i < list.size (); i++) {
			const ssi_char_t *filename = list.get (i);
			ssi_print ("converting '%s'..", filename);
			if (WavTools::ConvertVXtoWAV (filename)) {
				ssi_print ("ok\n");
			} else {
				ssi_print ("failed\n");
			}
		}
	} else {
		ssi_print ("converting '%s'..", filename);
		if (WavTools::ConvertVXtoWAV (filename)) {
			ssi_print ("ok\n");
		}  else {
			ssi_print ("failed\n");
		}
	}
}

void WAVtoVX (const ssi_char_t *filename, bool list_input) {

	ssi_print ("convert=WAVtoVX\nlist=%s\n\n", list_input ? "true" : "false");

	if (list_input) {
				
		StringList list;		
		FileTools::ReadFilesFromFile (list, filename);		
		for (ssi_size_t i = 0; i < list.size (); i++) {
			const ssi_char_t *filename = list.get (i);
			ssi_print ("converting '%s'..", filename);
			if (WavTools::ConvertWAVtoVX (filename)) {
				ssi_print ("ok\n");
			} else {
				ssi_print ("failed\n");
			}
		}
	} else {
		ssi_print ("converting '%s'..", filename);
		if (WavTools::ConvertWAVtoVX (filename)) {
			ssi_print ("ok\n");
		}  else {
			ssi_print ("failed\n");
		}
	}
}

void VXRepair (const ssi_char_t *filename, bool list_input) {

	ssi_print ("convert=VXRepair\nlist=%s\n\n", list_input ? "true" : "false");

	FilePath filepath (filename);
	ssi_char_t outdir[SSI_MAX_CHAR];
	ssi_sprint (outdir, "%srepaired\\", filepath.getDir ());
	ssi_mkdir (outdir);

	if (list_input) {
				
		StringList list;
		FileTools::ReadFilesFromFile (list, filename);
		for (ssi_size_t i = 0; i < list.size (); i++) {
			const ssi_char_t *filename = list.get (i);
			ssi_print ("converting '%s'..", filename);			
			if (ssi_exists (filename)) {
				ssi_stream_t data;				
				if (FileTools::RepairStreamFile (File::BINARY, filename, data)) {
					ssi_char_t *filename_new = ssi_strcat (outdir, filename);
					FileTools::WriteStreamFile (File::BINARY, filename_new, data);		
					delete[] filename_new;
				}
				ssi_stream_destroy (data);
				ssi_print ("ok\n");
			} else {
				ssi_print ("failed\n");
			}
		}
	} else {
		ssi_print ("converting '%s'..", filename);
		if (ssi_exists (filename)) {
			ssi_stream_t data;				
				if (FileTools::RepairStreamFile (File::BINARY, filename, data)) {
					ssi_char_t *filename_new = ssi_strcat (outdir, filename);
					FileTools::WriteStreamFile (File::BINARY, filename_new, data);		
					delete[] filename_new;
				}
				ssi_stream_destroy (data);
				ssi_print ("ok\n");
		}  else {
			ssi_print ("failed\n");
		}
	}
}

void V2Repair (const ssi_char_t *filename, bool list_input, ssi_time_t sr, ssi_type_t type, ssi_size_t dim) {

	ssi_print ("convert=V2Repair\nlist=%s\n\n", list_input ? "true" : "false");

	FilePath filepath (filename);
	ssi_char_t outdir[SSI_MAX_CHAR];
	ssi_sprint (outdir, "%srepaired\\", filepath.getDir ());
	ssi_mkdir (outdir);

	if (list_input) {
				
		StringList list;
		FileTools::ReadFilesFromFile (list, filename);
		for (ssi_size_t i = 0; i < list.size (); i++) {
			const ssi_char_t *filename = list.get (i);
			ssi_print ("converting '%s'..", filename);		
			ssi_char_t *path_ex = ssi_strcat (filename, ".stream~");		
			if (ssi_exists (path_ex)) {
				ssi_stream_t data;
				ssi_stream_init (data, 0, dim, ssi_type2bytes (type), type, sr);
				if (FileTools::RepairStreamFileV2 (File::BINARY, path_ex, data)) {
					ssi_char_t *filename_new = ssi_strcat (outdir, filename);
					FileTools::WriteStreamFile (File::BINARY, filename_new, data);		
					delete[] filename_new;
				}
				ssi_stream_destroy (data);
				ssi_print ("ok\n");
			} else {
				ssi_print ("failed\n");
			}
			delete[] path_ex;
		}
	} else {
		ssi_print ("converting '%s'..", filename);
		ssi_char_t *path_ex = ssi_strcat (filename, ".stream~");		
		if (ssi_exists (path_ex)) {
			ssi_stream_t data;			
			ssi_stream_init (data, 0, dim, ssi_type2bytes (type), type, sr);			
			if (FileTools::RepairStreamFileV2 (File::BINARY, path_ex, data)) {
				ssi_char_t *filename_new = ssi_strcat (outdir, filepath.getName ());
				FileTools::WriteStreamFile (File::BINARY, filename_new, data);		
				delete[] filename_new;
			}
			ssi_stream_destroy (data);
			ssi_print ("ok\n");
		}  else {
			ssi_print ("failed\n");
		}
		delete[] path_ex;
	}
}

void WAVRepair (const ssi_char_t *filename, bool list_input) {

	ssi_print ("convert=WAVRepair\nlist=%s\n\n", list_input ? "true" : "false");

	FilePath filepath (filename);
	ssi_char_t outdir[SSI_MAX_CHAR];
	ssi_sprint (outdir, "%srepaired\\", filepath.getDir ());
	ssi_mkdir (outdir);

	if (list_input) {
				
		StringList list;
		FileTools::ReadFilesFromFile (list, filename);
		for (ssi_size_t i = 0; i < list.size (); i++) {
			const ssi_char_t *filename = list.get (i);
			ssi_print ("converting '%s'..", filename);			
			if (ssi_exists (filename)) {
				ssi_stream_t data;				
				if (WavTools::RepairWavFile (filename, data)) {
					ssi_char_t *filename_new = ssi_strcat (outdir, filename);
					WavTools::WriteWavFile (filename_new, data);		
					delete[] filename_new;
				}
				ssi_stream_destroy (data);
				ssi_print ("ok\n");
			} else {
				ssi_print ("failed\n");
			}
		}
	} else {
		ssi_print ("converting '%s'..", filename);
		if (ssi_exists (filename)) {
			ssi_stream_t data;				
				if (WavTools::RepairWavFile (filename, data)) {
					ssi_char_t *filename_new = ssi_strcat (outdir, filepath.getNameFull ());
					WavTools::WriteWavFile (filename_new, data);		
					delete[] filename_new;
				}
				ssi_stream_destroy (data);
				ssi_print ("ok\n");
		}  else {
			ssi_print ("failed\n");
		}
	}
}
	
void VXtoCSV (const ssi_char_t *filename, bool list_input, bool csv_header) {

	ssi_print ("convert=VXtoCSV\nlist=%s\n\n", list_input ? "true" : "false");
	ssi_char_t string[SSI_MAX_CHAR];
	ssi_stream_t stream;

	if (list_input) {
				
		StringList list;		
		FileTools::ReadFilesFromFile (list, filename);		
		for (ssi_size_t i = 0; i < list.size (); i++) {
			const ssi_char_t *filename = list.get (i);
			ssi_print ("converting '%s'..", filename);
			if (FileTools::ReadStreamFile (filename, stream)) {
				FilePath fp (filename);
				ssi_sprint (string, "%s.csv", fp.getPath ());
				File *out = File::CreateAndOpen (File::ASCII, File::WRITE, string);
				if (csv_header) {
					out->writeLine("sep=,");
				}
				out->setFormat(",","");
				out->setType(stream.type);
				out->write(stream.ptr, stream.dim, stream.num * stream.dim);
				delete out;
				ssi_stream_destroy (stream);
				ssi_print ("ok\n");
			} else {
				ssi_print ("failed\n");
			}
		}
	} else {
		ssi_print ("converting '%s'..", filename);		
		if (FileTools::ReadStreamFile (filename, stream)) {
			FilePath fp (filename);
			ssi_sprint (string, "%s.csv", fp.getPath ());
			File *out = File::CreateAndOpen (File::ASCII, File::WRITE, string);
			if (csv_header) {
				out->writeLine("sep=,");
			}
			out->setFormat(",","");
			out->setType(stream.type);
			out->write(stream.ptr, stream.dim, stream.num * stream.dim);
			delete out;
			ssi_stream_destroy (stream);
			ssi_print ("ok\n");
		} else {
			ssi_print ("failed\n");
		}
	}

}
