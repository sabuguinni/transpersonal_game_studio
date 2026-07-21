#pragma once

// NarrativeVoiceRegistry.h
// Agent #15 — Narrative & Dialogue Agent
// Cycle: PROD_CYCLE_AUTO_20260629_008
// Registry mapping NPC speaker roles to their voice audio assets and generated TTS URLs.
// Used by the Audio Agent (#16) to load and play dialogue voice lines.

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "DialogueManager.h"
#include "NarrativeVoiceRegistry.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Voice entry — maps a speaker role to audio asset reference and TTS URL
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoiceEntry : public FTableRowBase
{
    GENERATED_BODY()

    // Speaker role this entry covers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    ENarr_SpeakerRole SpeakerRole = ENarr_SpeakerRole::Survivor;

    // Display name of the character (e.g., "Kael", "Elder Mara")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    FString CharacterName;

    // Optional UE5 SoundBase asset reference (set in editor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    TSoftObjectPtr<USoundBase> VoiceAsset;

    // Supabase/CDN URL for the generated TTS audio (from ElevenLabs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    FString TTS_URL;

    // Voice pitch modifier (-1.0 = low, 0.0 = normal, 1.0 = high)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float PitchModifier = 0.0f;

    // Volume scale for this character's voice
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float VolumeScale = 1.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// Produced voice lines this cycle — audio URLs from ElevenLabs TTS
// These are referenced here so Audio Agent (#16) can load them.
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ProducedVoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    ENarr_SpeakerRole SpeakerRole = ENarr_SpeakerRole::Survivor;
};

// ─────────────────────────────────────────────────────────────────────────────
// NarrativeVoiceRegistry — UObject that holds all voice mappings
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeVoiceRegistry : public UObject
{
    GENERATED_BODY()

public:
    UNarrativeVoiceRegistry();

    // All registered voice entries (populated from DataTable or manually)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    TArray<FNarr_VoiceEntry> VoiceEntries;

    // Voice lines produced this cycle (cycle 008) — for Audio Agent handoff
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative|Voice")
    TArray<FNarr_ProducedVoiceLine> Cycle008VoiceLines;

    // Get voice entry for a given speaker role
    UFUNCTION(BlueprintCallable, Category = "Narrative|Voice")
    bool GetVoiceEntry(ENarr_SpeakerRole Role, FNarr_VoiceEntry& OutEntry) const;

    // Register a new voice entry at runtime
    UFUNCTION(BlueprintCallable, Category = "Narrative|Voice")
    void RegisterVoiceEntry(const FNarr_VoiceEntry& Entry);

    // Populate with default entries for all speaker roles
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Narrative|Voice")
    void PopulateDefaults();

    // Get the TTS URL for a given character name
    UFUNCTION(BlueprintCallable, Category = "Narrative|Voice")
    FString GetTTSURL(const FString& CharacterName) const;
};
