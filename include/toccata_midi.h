#ifndef TOCCATA_MIDI_FILE_H
#define TOCCATA_MIDI_FILE_H

#include <Windows.h>

#include <fstream>

#include <yds_dynamic_array.h>

struct MidiChunkHeader
{

	BYTE Type[4];
	UINT32 Length;

};

// Formats
#define FORMAT_0 0x0000
#define FORMAT_1 0x0001
#define FORMAT_2 0x0002

struct alignas(2) MidiHeaderChunkData
{

	UINT16 Format;
	UINT16 Ntrks;
	UINT16 Division;

};

struct alignas(2) MidiMTrkEventHeader
{

	UINT16 DeltaTime;

};

class MidiTrack;
class MidiPianoSegment;
class MidiNote : public ysDynamicArrayElement
{

protected:

	enum NOTE_CLASS_TYPE
	{

		NOTE_CLASS_UNKNOWN,
		NOTE_CLASS_STANDARD,
		NOTE_CLASS_ANALYSIS,

	};

public:

	MidiNote();
	MidiNote(NOTE_CLASS_TYPE classType);
	virtual ~MidiNote();

	BYTE m_midiKey;
	BYTE m_velocity;

	uint64_t m_realTime;
	int m_noteLength;
	int m_timeStamp;

	void GetNaturalNotation(char *letter, bool *accidental, int *octave);
	void PrintDebug(int tempo = 0);

	void SetParent(MidiTrack *parent) { m_parent = parent; }
	MidiTrack *GetParent() { return m_parent; }

	MidiPianoSegment *GetSegment();

	void SetValid(bool valid) { m_valid = valid; }
	bool GetValid() const { return m_valid; }

	int GetTimeStampMilliseconds(int offset, int tempo) const;

	void Copy(const MidiNote *source) { source->CopyTo(this); }
	virtual void CopyTo(MidiNote *target) const;

	NOTE_CLASS_TYPE GetClassType() const { return m_classType; }

protected:

	bool m_valid;
	MidiTrack *m_parent;

private:

	NOTE_CLASS_TYPE m_classType;

};

class MidiAnalysisNote : public MidiNote
{

public:

	enum NOTE_TYPE
	{

		NOTE_UNKNOWN,
		NOTE_REFERENCE,
		NOTE_PLAYED

	};

	enum NOTE_ERROR_TYPE
	{

		NOTE_ERROR_NONE,
		NOTE_ERROR_MISSED,
		NOTE_ERROR_WRONG,
		NOTE_ERROR_NONSENSE

	};

public:

	MidiAnalysisNote();
	virtual ~MidiAnalysisNote();

	double m_timingError;
	double m_realTimeError;
	
	NOTE_TYPE m_type;
	NOTE_ERROR_TYPE m_errorType;

	virtual void CopyTo(MidiNote *midiNote) const;

};

class MidiTrack
{

	friend class MidiPianoSegment;

public:

	MidiTrack();
	~MidiTrack();

	void PrintDebug(int tempo = 0);

	MidiNote *AddNote();
	MidiAnalysisNote *AddAnalysisNote();
	void TransferNote(MidiNote *note, MidiTrack *target);
	void DeleteNote(MidiNote *note);

	MidiNote *FindLastNote(int midiKey, int timeStamp);
	MidiNote *FindClosestNote(int midiKey, int timeStamp, int timeThreshold, int startIndex = 0);

	MidiPianoSegment *GetSegment() { return m_parent; }
	void SetSegment(MidiPianoSegment *segment) { m_parent = segment; }

	bool IsEmpty() const { return m_notes.GetNumObjects() > 0; }
	void Clear() { m_notes.Clear(); }

	MidiNote **GetBuffer() { return m_notes.GetBuffer(); }
	int GetNoteCount() const { return m_notes.GetNumObjects(); }

	int GetStartDeltaTime() const;
	int GetEndDeltaTime() const;

	void MergeTrack(MidiTrack *alternateTrack);

protected:

	MidiPianoSegment *m_parent;
	ysDynamicArray<MidiNote, 4> m_notes;

};

class MidiPianoSegment : public ysDynamicArrayElement
{

	friend class MidiFile;

public:

	enum TIME_FORMAT
	{


		TIME_FORMAT_METRICAL,
		TIME_FORMAT_TIME_CODE,

	};

public:

	MidiPianoSegment();
	~MidiPianoSegment();

	MidiTrack m_leftHand;
	MidiTrack m_rightHand;

	int ConvertDeltaTimeToNote(int deltaTime) const;
	int ConvertDeltaTimeToMilliseconds(int deltaTime, int tempoBPM) const;
	int ConvertMillisecondsToDeltaTime(int ms, int tempoBPM) const;
	int ConvertSecondsToDeltaTime(double s, int tempoBPM) const;

	int GetStartDeltaTime() const;
	int GetEndDeltaTime() const;

	void SetTimeFormat(TIME_FORMAT timeFormat) { m_timeFormat = timeFormat; }
	TIME_FORMAT GetTimeFormat() const { return m_timeFormat; }

	void SetTicksPerQuarterNote(int value) { m_ticksPerQuarterNote = value; }
	void SetNegativeSMPTEFormat(int value) { m_negativeSMPTEFormat = value; }
	void SetTicksPerFrame(int value) { m_ticksPerFrame = value; }

	void SetTimeSignature(int numerator, int denominator) { m_timeSignatureNumerator = numerator;  m_timeSignatureDenominator = denominator; }
	int GetTimeSignatureDenominator() const { return m_timeSignatureDenominator; }
	int GetTimeSignatureNumerator() const { return m_timeSignatureNumerator; }

	void SetTempo(int tempo) { m_tempo = tempo; }
	void SetTempoBPM(int bpm);
	int GetTempo() const { return m_tempo; }
	int GetTempoBPM() const;

	bool IsEmpty() const { return (m_leftHand.IsEmpty() && m_rightHand.IsEmpty()); }

	void CopySettingsTo(MidiPianoSegment *targetSegment) const;

protected:

	TIME_FORMAT m_timeFormat;

	union
	{

		int m_ticksPerQuarterNote;

		struct
		{

			int m_negativeSMPTEFormat;
			int m_ticksPerFrame;

		};

	};

	// Tempo in microseconds per MIDI quarter-note
	int m_tempo;

	// Time Signature
	int m_timeSignatureNumerator;
	int m_timeSignatureDenominator;

};

class MidiChunk_Track;
class MidiFile
{

public:

	MidiFile();
	~MidiFile();

	void Write(const char *fname, MidiPianoSegment *segment);
	MidiPianoSegment *Read(const char *fname);

	static void ByteSwap32(UINT32 *data);
	static void ByteSwap24(UINT32 *data);
	static void ByteSwap16(UINT16 *data);

protected:

	void WriteTrackData(MidiChunk_Track *target, MidiTrack *source, MidiPianoSegment *segment, int channel);

	void ReadChunk(MidiChunkHeader *header);
	UINT32 ReadTrackEvent(UINT32 currentTime);

	UINT32 ReadVariableLengthQuantity();

	std::fstream m_file;

	MidiTrack *m_targetTrack;
	MidiPianoSegment *m_generatedSegment;
	unsigned int m_chunkDataRemaining;

protected:

	// Midi Data
	int m_nTracks;
	int m_format;

};

#endif