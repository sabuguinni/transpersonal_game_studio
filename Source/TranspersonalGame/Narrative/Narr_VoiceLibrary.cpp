#include "Narr_VoiceLibrary.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UNarr_VoiceLibrary::UNarr_VoiceLibrary()
{
    // Initialize with default voice clips
    InitializeDefaultVoiceClips();
}

FNarr_VoiceClip UNarr_VoiceLibrary::GetVoiceClip(const FString& ClipID) const
{
    for (const FNarr_VoiceClip& Clip : VoiceClips)
    {
        if (Clip.ClipID == ClipID)
        {
            return Clip;
        }
    }

    // Return empty clip if not found
    FNarr_VoiceClip EmptyClip;
    UE_LOG(LogTemp, Warning, TEXT("Voice clip not found: %s"), *ClipID);
    return EmptyClip;
}

TArray<FNarr_VoiceClip> UNarr_VoiceLibrary::GetVoiceClipsByType(ENarr_CharacterVoiceType VoiceType) const
{
    TArray<FNarr_VoiceClip> FilteredClips;

    for (const FNarr_VoiceClip& Clip : VoiceClips)
    {
        if (Clip.VoiceType == VoiceType)
        {
            FilteredClips.Add(Clip);
        }
    }

    return FilteredClips;
}

FNarr_VoiceClip UNarr_VoiceLibrary::GetRandomVoiceClip(ENarr_CharacterVoiceType VoiceType) const
{
    TArray<FNarr_VoiceClip> TypeClips = GetVoiceClipsByType(VoiceType);

    if (TypeClips.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No voice clips found for type: %d"), (int32)VoiceType);
        return FNarr_VoiceClip();
    }

    int32 RandomIndex = UKismetMathLibrary::RandomInteger(TypeClips.Num());
    return TypeClips[RandomIndex];
}

void UNarr_VoiceLibrary::AddVoiceClip(const FNarr_VoiceClip& NewClip)
{
    // Check if clip with same ID already exists
    for (int32 i = 0; i < VoiceClips.Num(); i++)
    {
        if (VoiceClips[i].ClipID == NewClip.ClipID)
        {
            VoiceClips[i] = NewClip; // Replace existing
            RebuildVoiceClipsByType();
            return;
        }
    }

    // Add new clip
    VoiceClips.Add(NewClip);
    RebuildVoiceClipsByType();

    UE_LOG(LogTemp, Log, TEXT("Added voice clip: %s"), *NewClip.ClipID);
}

bool UNarr_VoiceLibrary::RemoveVoiceClip(const FString& ClipID)
{
    for (int32 i = 0; i < VoiceClips.Num(); i++)
    {
        if (VoiceClips[i].ClipID == ClipID)
        {
            VoiceClips.RemoveAt(i);
            RebuildVoiceClipsByType();
            UE_LOG(LogTemp, Log, TEXT("Removed voice clip: %s"), *ClipID);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Voice clip not found for removal: %s"), *ClipID);
    return false;
}

void UNarr_VoiceLibrary::InitializeDefaultVoiceClips()
{
    VoiceClips.Empty();

    // Tribal Elder voice clips
    FNarr_VoiceClip ElderClip1;
    ElderClip1.ClipID = TEXT("elder_ancient_valley");
    ElderClip1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1780378268140_TribalElder.mp3");
    ElderClip1.TranscriptText = TEXT("The ancient valley holds many secrets, hunter. Follow the river downstream to the sacred gathering stones. There you will find others of our tribe who can teach you the old ways of survival.");
    ElderClip1.VoiceType = ENarr_CharacterVoiceType::TribalElder;
    ElderClip1.Duration = 13.0f;
    VoiceClips.Add(ElderClip1);

    // Scout voice clips
    FNarr_VoiceClip ScoutClip1;
    ScoutClip1.ClipID = TEXT("scout_danger_alert");
    ScoutClip1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1780378270264_Scout.mp3");
    ScoutClip1.TranscriptText = TEXT("Stay alert! The ground shakes with heavy footsteps. A massive beast approaches from the eastern cliffs. Keep low and move quietly through the tall grass until it passes.");
    ScoutClip1.VoiceType = ENarr_CharacterVoiceType::Scout;
    ScoutClip1.Duration = 12.0f;
    VoiceClips.Add(ScoutClip1);

    // Mentor voice clips
    FNarr_VoiceClip MentorClip1;
    MentorClip1.ClipID = TEXT("mentor_night_wisdom");
    MentorClip1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1780378272475_Mentor.mp3");
    MentorClip1.TranscriptText = TEXT("You have proven yourself worthy, young hunter. Take this knowledge - when the sun sets behind the great mountain, the night hunters emerge. Build your fire high and keep your weapons close.");
    MentorClip1.VoiceType = ENarr_CharacterVoiceType::Mentor;
    MentorClip1.Duration = 13.0f;
    VoiceClips.Add(MentorClip1);

    // Hunt Leader voice clips
    FNarr_VoiceClip HuntLeaderClip1;
    HuntLeaderClip1.ClipID = TEXT("huntleader_great_hunt");
    HuntLeaderClip1.AudioURL = TEXT("https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1780378274446_HuntLeader.mp3");
    HuntLeaderClip1.TranscriptText = TEXT("The tribe gathers at dawn for the great hunt. Bring your finest spear and strongest courage. We track the thunder lizard to the bone valley where our ancestors once walked.");
    HuntLeaderClip1.VoiceType = ENarr_CharacterVoiceType::HuntLeader;
    HuntLeaderClip1.Duration = 12.0f;
    VoiceClips.Add(HuntLeaderClip1);

    // Rebuild the type mapping
    RebuildVoiceClipsByType();

    UE_LOG(LogTemp, Log, TEXT("Initialized voice library with %d clips"), VoiceClips.Num());
}

TArray<FString> UNarr_VoiceLibrary::GetAllClipIDs() const
{
    TArray<FString> ClipIDs;
    for (const FNarr_VoiceClip& Clip : VoiceClips)
    {
        ClipIDs.Add(Clip.ClipID);
    }
    return ClipIDs;
}

void UNarr_VoiceLibrary::RebuildVoiceClipsByType()
{
    VoiceClipsByType.Empty();

    for (const FNarr_VoiceClip& Clip : VoiceClips)
    {
        if (!VoiceClipsByType.Contains(Clip.VoiceType))
        {
            VoiceClipsByType.Add(Clip.VoiceType, TArray<FString>());
        }
        VoiceClipsByType[Clip.VoiceType].Add(Clip.ClipID);
    }
}