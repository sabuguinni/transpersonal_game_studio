#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DataTable.h"
#include "Narr_VoiceActedNPCSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_NPCVoiceType : uint8
{
    ElderThorne         UMETA(DisplayName = "Elder Thorne - Survival Wisdom"),
    TrackerVera         UMETA(DisplayName = "Tracker Vera - Hunter Guide"),
    SurvivalNarrator    UMETA(DisplayName = "Survival Narrator - Environmental"),
    EmergencyAlert      UMETA(DisplayName = "Emergency Alert - Threat Warning"),
    ResourceScout       UMETA(DisplayName = "Resource Scout - Discovery"),
    WeatherObserver     UMETA(DisplayName = "Weather Observer - Climate Warning")
};

UENUM(BlueprintType)
enum class ENarr_DialogueContext : uint8
{
    Introduction        UMETA(DisplayName = "First Meeting"),
    DinosaurEncounter   UMETA(DisplayName = "Dinosaur Threat"),
    ResourceDiscovery   UMETA(DisplayName = "Resource Found"),
    WeatherWarning      UMETA(DisplayName = "Weather Alert"),
    SurvivalTip         UMETA(DisplayName = "Survival Advice"),
    EmergencyProtocol   UMETA(DisplayName = "Emergency Response"),
    QuestGiving         UMETA(DisplayName = "Quest Assignment"),
    QuestComplete       UMETA(DisplayName = "Quest Completion")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    ENarr_DialogueContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    int32 Priority;

    FNarr_VoiceLine()
    {
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        Context = ENarr_DialogueContext::Introduction;
        Priority = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCVoiceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    ENarr_NPCVoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    TArray<FNarr_VoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Profile")
    bool bCanInterrupt;

    FNarr_NPCVoiceProfile()
    {
        VoiceType = ENarr_NPCVoiceType::ElderThorne;
        CharacterName = TEXT("Unknown");
        InteractionRange = 300.0f;
        bCanInterrupt = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_VoiceActedNPC : public AActor
{
    GENERATED_BODY()

public:
    ANarr_VoiceActedNPC();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* VoiceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* NPCMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    FNarr_NPCVoiceProfile VoiceProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    bool bAutoPlayOnApproach;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Acting")
    float CooldownBetweenLines;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlayingVoiceLine;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastVoiceLineTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentVoiceLineIndex;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void PlayVoiceLineByContext(ENarr_DialogueContext Context);

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void PlayVoiceLineByIndex(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void StopCurrentVoiceLine();

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    bool CanPlayVoiceLine() const;

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    void AddVoiceLine(const FNarr_VoiceLine& NewVoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Voice Acting")
    TArray<FNarr_VoiceLine> GetVoiceLinesByContext(ENarr_DialogueContext Context) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice Acting")
    void OnVoiceLineStarted(const FNarr_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintImplementableEvent, Category = "Voice Acting")
    void OnVoiceLineCompleted(const FNarr_VoiceLine& VoiceLine);

protected:
    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                           bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void LoadAudioFromURL(const FString& AudioURL);
    void InitializeVoiceProfile();
    FNarr_VoiceLine SelectBestVoiceLineForContext(ENarr_DialogueContext Context) const;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarr_VoiceActedNPCManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "NPC Voice Management")
    void RegisterVoiceActedNPC(ANarr_VoiceActedNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "NPC Voice Management")
    void UnregisterVoiceActedNPC(ANarr_VoiceActedNPC* NPC);

    UFUNCTION(BlueprintCallable, Category = "NPC Voice Management")
    void PlayGlobalAnnouncement(ENarr_DialogueContext Context, ENarr_NPCVoiceType VoiceType);

    UFUNCTION(BlueprintCallable, Category = "NPC Voice Management")
    void StopAllVoiceLines();

    UFUNCTION(BlueprintCallable, Category = "NPC Voice Management")
    TArray<ANarr_VoiceActedNPC*> GetNPCsByVoiceType(ENarr_NPCVoiceType VoiceType) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Voice Management")
    bool IsAnyNPCSpeaking() const;

protected:
    UPROPERTY()
    TArray<ANarr_VoiceActedNPC*> RegisteredNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Management")
    bool bAllowSimultaneousVoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Management")
    float GlobalVoiceLineCooldown;

    float LastGlobalVoiceLineTime;
};