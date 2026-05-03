#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DataTable.h"
#include "NarrativeDialogueManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Research        UMETA(DisplayName = "Research Log"),
    Safety          UMETA(DisplayName = "Safety Warning"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Story           UMETA(DisplayName = "Story Narration"),
    Tutorial        UMETA(DisplayName = "Tutorial"),
    Environmental   UMETA(DisplayName = "Environmental")
};

UENUM(BlueprintType)
enum class ENarr_TriggerCondition : uint8
{
    PlayerProximity     UMETA(DisplayName = "Player Proximity"),
    DinosaurSighting    UMETA(DisplayName = "Dinosaur Sighting"),
    ResourceDiscovery   UMETA(DisplayName = "Resource Discovery"),
    DangerDetected      UMETA(DisplayName = "Danger Detected"),
    QuestProgress       UMETA(DisplayName = "Quest Progress"),
    TimeOfDay           UMETA(DisplayName = "Time of Day")
};

USTRUCT(BlueprintType)
struct FNarr_DialogueEntry : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_TriggerCondition TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanRepeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float TriggerRadius;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("DefaultDialogue");
        DialogueText = FText::FromString(TEXT("Default dialogue text"));
        DialogueType = ENarr_DialogueType::Story;
        TriggerCondition = ENarr_TriggerCondition::PlayerProximity;
        Duration = 5.0f;
        Priority = 1;
        bCanRepeat = false;
        CooldownTime = 30.0f;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct FNarr_ActiveDialogue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogue")
    FNarr_DialogueEntry DialogueData;

    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogue")
    float TimeRemaining;

    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogue")
    bool bIsPlaying;

    UPROPERTY(BlueprintReadOnly, Category = "Active Dialogue")
    float LastPlayedTime;

    FNarr_ActiveDialogue()
    {
        TimeRemaining = 0.0f;
        bIsPlaying = false;
        LastPlayedTime = 0.0f;
    }
};

/**
 * Gestor central de diálogos e narrativa dinâmica
 * Controla reprodução de áudio contextual baseado em eventos do jogo
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarrativeDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarrativeDialogueManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* DialogueAudioComponent;

    // Configuração
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    float MaxDialogueDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    bool bAutoTriggerDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Settings")
    int32 MaxConcurrentDialogues;

    // Estado do sistema
    UPROPERTY(BlueprintReadOnly, Category = "Narrative State")
    TArray<FNarr_ActiveDialogue> ActiveDialogues;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative State")
    TArray<FString> PlayedDialogueIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative State")
    FNarr_ActiveDialogue CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative State")
    bool bIsDialoguePlaying;

public:
    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool PlayDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogueByCondition(ENarr_TriggerCondition Condition, const FVector& Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueCompletion(const FString& DialogueID);

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool IsDialogueAvailable(const FString& DialogueID) const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    TArray<FNarr_DialogueEntry> GetDialoguesByType(ENarr_DialogueType DialogueType) const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    float GetDialogueProgress() const;

    // Eventos
    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative Events")
    void OnDialogueStarted(const FNarr_DialogueEntry& DialogueData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative Events")
    void OnDialogueCompleted(const FNarr_DialogueEntry& DialogueData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Narrative Events")
    void OnDialogueInterrupted(const FNarr_DialogueEntry& DialogueData);

protected:
    // Funções internas
    void UpdateActiveDialogues(float DeltaTime);
    void CheckTriggerConditions();
    void ProcessDialogueQueue();
    
    FNarr_DialogueEntry* FindDialogueByID(const FString& DialogueID);
    TArray<FNarr_DialogueEntry> GetTriggeredDialogues(ENarr_TriggerCondition Condition, const FVector& Location);
    
    bool CanPlayDialogue(const FNarr_DialogueEntry& DialogueData) const;
    void StartDialoguePlayback(const FNarr_DialogueEntry& DialogueData);
    void CompleteDialogue();

    // Detecção de contexto
    void CheckPlayerProximity();
    void CheckDinosaurSightings();
    void CheckResourceDiscoveries();
    void CheckDangerSituations();

    // Utilitários
    APawn* GetPlayerPawn() const;
    float GetDistanceToPlayer(const FVector& Location) const;
    bool IsLocationVisible(const FVector& Location) const;

private:
    // Cache
    UPROPERTY()
    APawn* CachedPlayer;

    // Timers
    float ProximityCheckTimer;
    float ContextCheckTimer;
    
    static constexpr float ProximityCheckInterval = 0.5f;
    static constexpr float ContextCheckInterval = 1.0f;

    // Queue de diálogos
    TArray<FNarr_DialogueEntry> DialogueQueue;
    
    // Cooldowns
    TMap<FString, float> DialogueCooldowns;
};