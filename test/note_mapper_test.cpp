#include <pch.h>

#include "../include/note_mapper.h"

TEST(NoteMapperTest, SanityCheck) {
	toccata::MusicSegment reference;

	reference.NoteContainer.AddPoint({ 0.0 });
	reference.NoteContainer.AddPoint({ 0.1 });
	reference.NoteContainer.AddPoint({ 0.2 });

	const int n = reference.NoteContainer.GetCount();
	const toccata::MusicPoint *points = reference.NoteContainer.GetPoints();

	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, -0.1, 0), 0);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.0, 0), 0);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.1, 0), 1);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.2, 0), 2);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.3, 0), 2);
}

TEST(NoteMapperTest, SinglePoint) {
	toccata::MusicSegment reference;

	reference.NoteContainer.AddPoint({ 0.0 });

	const int n = reference.NoteContainer.GetCount();
	const toccata::MusicPoint *points = reference.NoteContainer.GetPoints();

	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, -0.1, 0), 0);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.0, 0), 0);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.1, 0), 0);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.2, 0), 0);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.3, 0), 0);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.3, 1), -1);
}

TEST(NoteMapperTest, NoPoints) {
	toccata::MusicSegment reference;

	const int n = reference.NoteContainer.GetCount();
	const toccata::MusicPoint *points = reference.NoteContainer.GetPoints();

	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, -0.1, 0), -1);
}

TEST(NoteMapperTest, MultiplePitches) {
	toccata::MusicSegment reference;

	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 0.1, 0 });
	reference.NoteContainer.AddPoint({ 0.16, 1 });
	reference.NoteContainer.AddPoint({ 0.2, 0 });
	reference.NoteContainer.AddPoint({ 0.26, 1 });
	reference.NoteContainer.AddPoint({ 0.66, 1 });

	const int n = reference.NoteContainer.GetCount();
	const toccata::MusicPoint *points = reference.NoteContainer.GetPoints();

	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.0, 0), 0);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.16, 0), 3);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.26, 0), 3);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.66, 0), 3);

	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.0, 1), 2);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.16, 1), 2);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.26, 1), 4);
	EXPECT_EQ(toccata::NoteMapper::GetClosestNote(points, 0, n - 1, 0.66, 1), 5);
}

TEST(NoteMapperTest, TrivialMapping) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;

	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 0.1, 0 });
	reference.NoteContainer.AddPoint({ 0.16, 1 });
	reference.NoteContainer.AddPoint({ 0.2, 0 });
	reference.NoteContainer.AddPoint({ 0.26, 1 });
	reference.NoteContainer.AddPoint({ 0.66, 1 });

	toccata::NoteMapper::NNeighborMappingRequest request;
	request.CorrelationThreshold = 0.1;
	request.ReferenceSegment = &reference;
	request.Segment = &reference;
	request.Start = 0;
	request.End = 5;
	request.s = 1.0;
	request.t = 0.0;
	request.Target = new int[6];
	
	const int *mapping = toccata::NoteMapper::GetMapping(&request);

	for (int i = 0; i < 6; ++i) {
		EXPECT_EQ(mapping[i], i);
	}

	delete[] mapping;
}

TEST(NoteMapperTest, MappingWithOffset) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;

	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 0.1, 0 });
	reference.NoteContainer.AddPoint({ 0.16, 1 });
	reference.NoteContainer.AddPoint({ 0.2, 0 });
	reference.NoteContainer.AddPoint({ 0.26, 1 });
	reference.NoteContainer.AddPoint({ 0.66, 1 });

	toccata::NoteMapper::NNeighborMappingRequest request;
	request.CorrelationThreshold = 0.1;
	request.ReferenceSegment = &reference;
	request.Segment = &reference;
	request.Start = 0;
	request.End = 5;
	request.s = 1.0;
	request.t = 2.0;
	request.Target = new int[6];

	const int *mapping = toccata::NoteMapper::GetMapping(&request);

	for (int i = 0; i < 6; ++i) {
		EXPECT_EQ(mapping[i], -1);
	}

	delete[] mapping;
}

TEST(NoteMapperTest, MappingSinglePoint) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;
	reference.NoteContainer.AddPoint({ 0.0, 0 });

	toccata::MusicSegment segment;
	segment.Length = 5.0;
	segment.NoteContainer.AddPoint({ 1.0, 0 });

	toccata::NoteMapper::NNeighborMappingRequest request;
	request.CorrelationThreshold = 0.1;
	request.ReferenceSegment = &reference;
	request.Segment = &segment;
	request.Start = 0;
	request.End = 0;
	request.s = 1.0;
	request.t = -1.0;
	request.Target = new int[1];

	const int *mapping = toccata::NoteMapper::GetMapping(&request);
	EXPECT_EQ(mapping[0], 0);

	delete[] mapping;
}

TEST(NoteMapperTest, MappingScale) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;
	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 0.1, 0 });
	reference.NoteContainer.AddPoint({ 0.2, 0 });

	toccata::MusicSegment segment;
	segment.Length = 10.0;
	segment.NoteContainer.AddPoint({ 0.0, 0 });
	segment.NoteContainer.AddPoint({ 1.0, 0 });
	segment.NoteContainer.AddPoint({ 2.0, 0 });

	toccata::NoteMapper::NNeighborMappingRequest request;
	request.CorrelationThreshold = 0.1;
	request.ReferenceSegment = &reference;
	request.Segment = &segment;
	request.Start = 0;
	request.End = 2;
	request.s = 0.1;
	request.t = 0.0;
	request.Target = new int[3];

	const int *mapping = toccata::NoteMapper::GetMapping(&request);
	EXPECT_EQ(mapping[0], 0);
	EXPECT_EQ(mapping[1], 1);
	EXPECT_EQ(mapping[2], 2);

	delete[] mapping;
}

TEST(NoteMapperTest, MappingWrongScale) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;
	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 0.1, 0 });
	reference.NoteContainer.AddPoint({ 0.2, 0 });

	toccata::MusicSegment segment;
	segment.Length = 10.0;
	segment.NoteContainer.AddPoint({ 0.0, 0 });
	segment.NoteContainer.AddPoint({ 2.0, 0 });

	toccata::NoteMapper::NNeighborMappingRequest request;
	request.CorrelationThreshold = 0.1;
	request.ReferenceSegment = &reference;
	request.Segment = &segment;
	request.Start = 0;
	request.End = 1;
	request.s = 10.0;
	request.t = 0.0;
	request.Target = new int[3];

	const int *mapping = toccata::NoteMapper::GetMapping(&request);
	EXPECT_EQ(mapping[0], 0);
	EXPECT_EQ(mapping[1], -1);
	EXPECT_EQ(mapping[2], -1);

	delete[] mapping;
}

double **CreateCorrespondenceMatrix(int n, int m) {
	double **newArray = new double *[n];
	for (int i = 0; i < n; ++i) {
		newArray[i] = new double[m];
	}

	return newArray;
}

TEST(NoteMapperTest, InjectiveMappingSanityCheck) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;

	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 0.1, 0 });
	reference.NoteContainer.AddPoint({ 0.16, 1 });
	reference.NoteContainer.AddPoint({ 0.2, 0 });
	reference.NoteContainer.AddPoint({ 0.26, 1 });
	reference.NoteContainer.AddPoint({ 0.66, 1 });

	toccata::NoteMapper::InjectiveMappingRequest request;
	request.CorrelationThreshold = 0.1;
	request.ReferenceSegment = &reference;
	request.Segment = &reference;
	request.Start = 0;
	request.End = 5;
	request.s = 1.0;
	request.t = 0.0;
	request.Target = new int[6];

	toccata::NoteMapper::AllocateMemorySpace(&request.Memory, 6, 6);
	const int *mapping = toccata::NoteMapper::GetInjectiveMapping(&request);
	toccata::NoteMapper::FreeMemorySpace(&request.Memory);

	for (int i = 0; i < 6; ++i) {
		EXPECT_EQ(mapping[i], i);
	}

	delete[] mapping;
}

TEST(NoteMapperTest, InjectiveMappingWrongScale) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;
	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 0.1, 0 });
	reference.NoteContainer.AddPoint({ 0.2, 0 });

	toccata::MusicSegment segment;
	segment.Length = 10.0;
	segment.NoteContainer.AddPoint({ 0.0, 0 });
	segment.NoteContainer.AddPoint({ 2.0, 0 });

	toccata::NoteMapper::InjectiveMappingRequest request;
	request.CorrelationThreshold = 0.1;
	request.ReferenceSegment = &reference;
	request.Segment = &segment;
	request.Start = 0;
	request.End = 1;
	request.s = 10.0;
	request.t = 0.0;
	request.Target = new int[3];

	toccata::NoteMapper::AllocateMemorySpace(&request.Memory, 3, 2);
	const int *mapping = toccata::NoteMapper::GetInjectiveMapping(&request);
	toccata::NoteMapper::FreeMemorySpace(&request.Memory);

	EXPECT_EQ(mapping[0], 0);
	EXPECT_EQ(mapping[1], -1);
	EXPECT_EQ(mapping[2], -1);

	delete[] mapping;
}

TEST(NoteMapperTest, InjectiveMappingDiscernment) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;
	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 1.0, 0 });
	reference.NoteContainer.AddPoint({ 2.0, 0 });

	toccata::MusicSegment segment;
	segment.Length = 10.0;
	segment.NoteContainer.AddPoint({ 0.0, 0 });
	segment.NoteContainer.AddPoint({ 1.9, 0 });
	segment.NoteContainer.AddPoint({ 2.0, 0 });
	segment.NoteContainer.AddPoint({ 2.1, 0 });

	toccata::NoteMapper::InjectiveMappingRequest request;
	request.CorrelationThreshold = 0.1;
	request.ReferenceSegment = &reference;
	request.Segment = &segment;
	request.s = 1.0;
	request.t = 0.0;
	request.Start = 0;
	request.End = 3;
	request.Target = new int[3];

	toccata::NoteMapper::AllocateMemorySpace(&request.Memory, 3, 4);
	const int *mapping = toccata::NoteMapper::GetInjectiveMapping(&request); 
	toccata::NoteMapper::FreeMemorySpace(&request.Memory);

	EXPECT_EQ(mapping[0], 0);
	EXPECT_EQ(mapping[1], -1);
	EXPECT_EQ(mapping[2], 2);

	delete[] mapping;
}

TEST(NoteMapperTest, InjectiveMappingCompetition) {
	toccata::MusicSegment reference;
	reference.Length = 1.0;
	reference.NoteContainer.AddPoint({ 0.0, 0 });
	reference.NoteContainer.AddPoint({ 1.1, 0 });
	reference.NoteContainer.AddPoint({ 1.2, 0 });

	toccata::MusicSegment segment;
	segment.Length = 1.0;
	segment.NoteContainer.AddPoint({ 0.0, 0 });
	segment.NoteContainer.AddPoint({ 1.0, 0 });
	segment.NoteContainer.AddPoint({ 1.1, 0 });
	segment.NoteContainer.AddPoint({ 1.5, 0 });

	toccata::NoteMapper::InjectiveMappingRequest request;
	request.CorrelationThreshold = 0.15;
	request.ReferenceSegment = &reference;
	request.Segment = &segment;
	request.Start = 0;
	request.End = 3;
	request.s = 1.0;
	request.t = 0.0;
	request.Target = new int[3];

	toccata::NoteMapper::AllocateMemorySpace(&request.Memory, 3, 4);
	const int *mapping = toccata::NoteMapper::GetInjectiveMapping(&request);
	toccata::NoteMapper::FreeMemorySpace(&request.Memory);

	EXPECT_EQ(mapping[0], 0);
	EXPECT_EQ(mapping[1], 1);
	EXPECT_EQ(mapping[2], 2);

	delete[] mapping;
}