#include "toccata_recorder.h"

#include "toccata_core.h"

Toccata_InputBuffer::Toccata_InputBuffer()
{
}

Toccata_InputBuffer::~Toccata_InputBuffer()
{
}

Toccata_RawMidiFileOutput::Toccata_RawMidiFileOutput()
{
}

Toccata_RawMidiFileOutput::~Toccata_RawMidiFileOutput()
{
}

void Toccata_RawMidiFileOutput::Open(const char *fname)
{

	m_file.open(fname, std::ios::out);

}

void Toccata_RawMidiFileOutput::WriteLine(DWORD wMsg, DWORD dwParam1, DWORD dwParam2)
{

	m_file << wMsg << " ";
	m_file << dwParam1 << " ";
	m_file << dwParam2 << "\n";

}

Toccata_RawMidiFileInput::Toccata_RawMidiFileInput()
{
}

Toccata_RawMidiFileInput::~Toccata_RawMidiFileInput()
{
}

bool Toccata_RawMidiFileInput::ReadLine(DWORD *wMsg, DWORD *dwParam1, DWORD *dwParam2)
{

	if (m_file.eof()) return false;

	m_file >> *wMsg;
	m_file >> *dwParam1;
	m_file >> *dwParam2;

	return true;

}

void Toccata_RawMidiFileInput::Open(const char *fname)
{

	m_file.open(fname, std::ios::in);

}

Toccata_Recorder::Toccata_Recorder()
{

	// Create the first recording
	NewSegment();

}

Toccata_Recorder::~Toccata_Recorder()
{
}

void Toccata_Recorder::Initialize()
{

	std::string fname = Toccata.GetDirectory();
	fname += "/raw_recording.rmid";

	m_rawOutput.Open(fname.c_str());

}

void Toccata_Recorder::OnNewTempo()
{

	bool metronomeEnabled = m_core->IsMetronomeEnabled();
	int tempo = m_core->GetTempoBPM();

	MidiPianoSegment *newSegment = NULL;
	
	if (m_currentTarget->IsEmpty())
	{

		// Just reuse the current segment
		newSegment = m_currentTarget;
		m_midiTimeStampStart = 0;
		m_currentMidiTimeStamp = 0;
		m_currentMidiTimeStampExternal = 0;
		m_recording = false;

	}

	else
	{

		// Create a new segment
		newSegment = NewSegment();
		m_midiTimeStampStart = 0;
		m_currentMidiTimeStamp = 0;
		m_currentMidiTimeStampExternal = 0;
		m_recording = false;

	}

	newSegment->SetTimeFormat(MidiPianoSegment::TIME_FORMAT_TIME_CODE);

	if (tempo == 0 || !metronomeEnabled)
	{
	
		// Assume 120 bpm, 4/4 time

		newSegment->SetTempoBPM(120);

		// 0.5 sec/whole note
		// 0.125 sec/quarter note
		// 1250 ms/quarter note

		newSegment->SetTicksPerQuarterNote(1250);

	}

	else
	{

		newSegment->SetTempoBPM(m_core->GetTempoBPM());

		// Calculate number of ticks needed for millisecond accuracy
		double period = (60.0 / m_core->GetTempoBPM()) / 4;
		period *= 1000; // Convert to ms

		newSegment->SetTicksPerQuarterNote((int)(period + 0.5) * 100);

	}

	int timeNumerator = m_core->GetTime();
	int timeDenominator = 2;

	newSegment->SetTimeSignature(timeNumerator, timeDenominator);

}

void Toccata_Recorder::OnNewTime()
{

	OnNewTempo();

}

void Toccata_Recorder::IncrementTime(double dt)
{

	if (m_recording)
	{

		//m_deltaTime += m_currentTarget->ConvertSecondsToDeltaTime(dt, m_currentTarget->GetTempo());

	}

}

int Toccata_Recorder::OutstandingNoteCount()
{

	int nNotes = m_inputBuffer.GetTrack()->GetNoteCount();
	int outstandingCount = 0;

	for (int i = 0; i < nNotes; i++)
	{

		if (!m_inputBuffer.GetNotes()[i]->GetValid()) outstandingCount++;

	}

	return outstandingCount;

}

int Toccata_Recorder::ProcessInputBuffer(int *nOutstanding)
{

	m_inputBuffer.LockBuffer();

	int nNotes = m_inputBuffer.GetTrack()->GetNoteCount();
	MidiNote **notes;

	int newNotes = 0;

	for (int i = 0; i < nNotes; i++)
	{

		// The notes array MAY be resized in this loop
		notes = m_inputBuffer.GetTrack()->GetBuffer();

		if (notes[i]->GetValid())
		{

			if (notes[i]->m_timeStamp > m_currentMidiTimeStamp)
			{

				m_currentMidiTimeStamp = notes[i]->m_timeStamp;

			}

			//notes[i]->PrintDebug(m_core->GetTempo());

			// Transfer the note to the right hand track
			m_inputBuffer.GetTrack()->TransferNote(notes[i], &m_currentTarget->m_rightHand);

			// One note has been removed from the input buffer track
			i--;
			nNotes--;
			newNotes++;

		}

		else
		{

			// Nothing after this point is valid
			break;

		}

	}

	m_currentMidiTimeStampExternal = m_currentMidiTimeStamp;

	// Get the number of outstanding notes before the buffer is unlocked
	*nOutstanding = OutstandingNoteCount();

	m_inputBuffer.UnlockBuffer();

	// Return the number of new notes to process
	return newNotes;

}

void Toccata_Recorder::ProcessMidiTick(int timeStamp)
{

	m_inputBuffer.LockBuffer();

	int relativeTimeStamp = timeStamp - m_midiTimeStampStart;

	if (OutstandingNoteCount() == 0)
	{

		relativeTimeStamp = m_currentTarget->ConvertMillisecondsToDeltaTime(relativeTimeStamp, m_currentTarget->GetTempoBPM());

		if (relativeTimeStamp > m_currentMidiTimeStamp)
		{

			m_currentMidiTimeStamp = relativeTimeStamp;

		}

	}

	else
	{

		// Future cannot be reasonably declared

	}

	m_inputBuffer.UnlockBuffer();

}

void Toccata_Recorder::ProcessNote(int midiKey, int velocity, int timeStamp, uint64_t systemTimeStamp)
{

	m_inputBuffer.LockBuffer();

	// If this is the first key-press, then start the recording
	if (m_currentTarget->IsEmpty() && velocity > 0)
	{

		m_midiTimeStampStart = timeStamp;
		m_recording = true;

	}

	int relativeTimeStamp = timeStamp - m_midiTimeStampStart;

	if (velocity > 0)
	{

		// Note was pressed

		MidiNote *newNote = m_inputBuffer.NewNote();

		newNote->m_realTime = systemTimeStamp;
		newNote->m_timeStamp = m_currentTarget->ConvertMillisecondsToDeltaTime(relativeTimeStamp, m_currentTarget->GetTempoBPM());
		newNote->m_midiKey = midiKey;
		newNote->m_velocity = velocity;
		newNote->SetValid(false);

	}

	else
	{

		// Note was released

		int currentTime = m_currentTarget->ConvertMillisecondsToDeltaTime(relativeTimeStamp, m_currentTarget->GetTempoBPM());
		MidiNote *lastNote = m_inputBuffer.GetTrack()->FindLastNote(midiKey, currentTime);

		if (lastNote != NULL)
		{

			lastNote->m_noteLength = m_currentTarget->ConvertMillisecondsToDeltaTime(relativeTimeStamp, m_currentTarget->GetTempoBPM()) - lastNote->m_timeStamp;
			lastNote->SetValid(true);

		}

		else
		{

			// This was likely a key released before this track began recording

		}

	}

	m_inputBuffer.UnlockBuffer();

}

MidiPianoSegment *Toccata_Recorder::NewSegment()
{

	MidiPianoSegment *newSegment = m_recordedSegments.New();

	m_currentTarget = newSegment;
	m_midiTimeStampStart = 0;
	
	m_inputBuffer.LockBuffer();
	m_inputBuffer.GetTrack()->Clear();
	m_inputBuffer.GetTrack()->SetSegment(newSegment);
	m_inputBuffer.UnlockBuffer();

	return newSegment;

}