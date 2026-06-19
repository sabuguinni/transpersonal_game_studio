#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AudioZoneManager.generated.h"

// ============================================================
// Audio Zone Manager — Agent #16 Audio Agent
// Manages positional ambient audio zones for MinPlayableMap.
// Zones: TribeCamp, RaptorDen, River, ForestEdge, Plains
// Wires dialogue audio cue IDs from DialogueManager to MetaSounds.
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    None        UMETA(DisplayName = "None"),
    TribeCamp   UMETA(DisplayName = "Tribe Camp"),
    RaptorDen   UMETA(DisplayName = "Raptor Den"),
    River       UMETA(DisplayName = "River"),
    ForestEdge  UMETA(DisplayName = "Forest Edge"),
    Plains      UMETA(DisplayName = "Plains"),
};

USTRUCT(BlueprintType)
struct FAudio_ZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName ZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BlendRadius = 800.0f;

    // Freesound asset IDs for this zone's ambient layer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<int32> FreesoundAssetIDs;

    // Dialogue audio cue IDs that play in this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FName> DialogueCueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueCueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName CueID;

    // Supabase TTS URL
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName OwnerZoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudio_ZoneEntered, FName, ZoneID, EAudio_ZoneType, ZoneType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudio_ZoneExited, FName, ZoneID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudio_DialogueCuePlayed, FName, CueID);

UCLASS()
class TRANSPERSONALGAME_API UAudioZoneManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    // --- Zone Registration ---
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterZone(const FAudio_ZoneData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UnregisterZone(FName ZoneID);

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool GetZoneData(FName ZoneID, FAudio_ZoneData& OutData) const;

    // --- Player Position Tracking ---
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdatePlayerLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintPure, Category = "Audio")
    FName GetActiveZoneID() const { return ActiveZoneID; }

    UFUNCTION(BlueprintPure, Category = "Audio")
    EAudio_ZoneType GetActiveZoneType() const;

    // --- Dialogue Audio Cue Wiring ---
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterDialogueCue(const FAudio_DialogueCueEntry& CueEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueCue(FName CueID);

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool GetDialogueCue(FName CueID, FAudio_DialogueCueEntry& OutEntry) const;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Audio")
    FOnAudio_ZoneEntered OnZoneEntered;

    UPROPERTY(BlueprintAssignable, Category = "Audio")
    FOnAudio_ZoneExited OnZoneExited;

    UPROPERTY(BlueprintAssignable, Category = "Audio")
    FOnAudio_DialogueCuePlayed OnDialogueCuePlayed;

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:

    UPROPERTY()
    TMap<FName, FAudio_ZoneData> RegisteredZones;

    UPROPERTY()
    TMap<FName, FAudio_DialogueCueEntry> DialogueCues;

    FName ActiveZoneID = NAME_None;
    FVector LastPlayerLocation = FVector::ZeroVector;

    void CheckZoneTransition(FVector PlayerLocation);
    FName FindNearestZone(FVector PlayerLocation) const;
};
