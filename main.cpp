#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

//Serializer struct 
struct lbp_serializer
{
	int DataVersion;
	FILE* FilePtr;
	bool IsWriting;
};

//Enum For the Versions
enum : int32_t
{
	SV_INITIAL = 1,
	SV_InitV2,
	SV_RemoveP3P4Score,
	// Don't remove this
	SV_LatestPlusOne
};

#define SV_LatestVersion (SV_LatestPlusOne - 1)

void Serialize(lbp_serializer* LbpSerializer, int* Datum)
{
	if (LbpSerializer->IsWriting)
	{
		fwrite(Datum, sizeof(int), 1, LbpSerializer->FilePtr);
	}
	else
	{
		fread(Datum, sizeof(int), 1, LbpSerializer->FilePtr);
	}
}

//Data struct Exemple
struct game_score_state
{
	int P1Score;
	int P2Score;
	//int P3Score; //remove on the 3rd versions
	//int P4Score;	//remove on the 3rd versions
	int P5Score;
};

//Add
#define ADD(_fieldAdded, _fieldName) \
    if (LbpSerializer->DataVersion >= (_fieldAdded)) \
    { \
        Serialize(LbpSerializer, _fieldName); \
    }

//Remove
#define REM(_fieldAdded, _fieldRemoved, _type, _fieldName, _defaultValue) \
    _type _fieldName = (_defaultValue); \
    if (LbpSerializer->DataVersion >= (_fieldAdded) && LbpSerializer->DataVersion < (_fieldRemoved)) \
    { \
        Serialize(LbpSerializer, &(_fieldName)); \
    }

//Serialize for the Struct we created
void Serialize(lbp_serializer* LbpSerializer, game_score_state* Datum)
{
	ADD(SV_INITIAL, &Datum->P1Score);
	ADD(SV_INITIAL, &Datum->P2Score);
	REM(SV_InitV2, SV_RemoveP3P4Score, int, P3Score, 0);
	REM(SV_InitV2, SV_RemoveP3P4Score, int, P4Score, 0);
	ADD(SV_RemoveP3P4Score, &Datum->P5Score);
}

bool SerializeIncludingVersion(lbp_serializer* LbpSerializer, game_score_state* State)
{
	if (LbpSerializer->IsWriting)
	{
		LbpSerializer->DataVersion = SV_LatestVersion;
	}

	Serialize(LbpSerializer, &LbpSerializer->DataVersion);

	// We are reading a file from a version that came after this one!
	if (LbpSerializer->DataVersion > SV_LatestVersion)
	{
		return false;
	}
	else
	{
		Serialize(LbpSerializer, State);
		return true;
	}
}


int main()
{
	lbp_serializer newSerialize;
	//score we write
	game_score_state currentScore;
	currentScore.P1Score = 120;
	currentScore.P2Score = 210;
	currentScore.P5Score = 90;
	
	newSerialize.IsWriting = true;

	newSerialize.FilePtr = fopen("myfile.bin", "wb");
	SerializeIncludingVersion(&newSerialize, &currentScore);
	fclose(newSerialize.FilePtr);

	printf("Save \n");

	//New score we read
	game_score_state SavedScore;

	newSerialize.IsWriting = false;
	newSerialize.FilePtr = fopen("myfile.bin", "rb");
	SerializeIncludingVersion(&newSerialize, &SavedScore);
	fclose(newSerialize.FilePtr);
	printf("READ : \n P1 score: %i \n P2 score: %i \n P5 score: %i", SavedScore.P1Score, SavedScore.P2Score, SavedScore.P5Score);
	return 0;

}