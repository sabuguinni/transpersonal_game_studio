#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Core/SharedTypes.h"
#include "Narr_DialogueSystem.generated.h"

/**
 * NARRATIVE AGENT #15 - DIALOGUE SYSTEM
 * 
 * Sistema de diálogo contextual para o jogo de sobrevivência pré-histórico.
 * Gere conversas baseadas em situações de sobrevivência, observação científica
 * e interacções com o ambiente perigoso do Cretáceo.
 * 
 * CARACTERÍSTICAS:
 * - Diálogos contextuais baseados em perigo, recursos, descobertas
 * - Sistema de narração para logs de pesquisa e observação
 * - Avisos de sobrevivência em tempo real
 * - Progressão narrativa baseada em competência de sobrevivência
 */

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    ResearchLog = 0         UMETA(DisplayName = "Research Log"),
    SurvivalWarning = 1     UMETA(DisplayName = "Survival Warning"),
    Discovery = 2           UMETA(DisplayName = "Discovery"),
    Objective = 3           UMETA(DisplayName = "Objective"),
    Emergency = 4           UMETA(DisplayName = "Emergency"),
    Environmental = 5       UMETA(DisplayName = "Environmental")
};

UENUM(BlueprintType)
enum class ENarr_SpeakerType : uint8
{
    FieldResearcher = 0     UMETA(DisplayName = "Field Researcher"),
    SurvivalGuide = 1       UMETA(DisplayName = "Survival Guide"),
    MissionController = 2   UMETA(DisplayName = "Mission Controller"),
    SurvivorNarrator = 3    UMETA(DisplayName = "Survivor Narrator"),
    EnvironmentNarrator = 4 UMETA(DisplayName = "Environment Narrator")
};

UENUM(BlueprintType)
enum class ENarr_UrgencyLevel : uint8
{
    Info = 0                UMETA(DisplayName = "Info"),
    Warning = 1             UMETA(DisplayName = "Warning"),
    Danger = 2              UMETA(DisplayName = "Danger"),
    Critical = 3            UMETA(DisplayName = "Critical"),
    Emergency = 4           UMETA(DisplayName = "Emergency")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_SpeakerType Speaker = ENarr_SpeakerType::FieldResearcher;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType Type = ENarr_DialogueType::ResearchLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_UrgencyLevel Urgency = ENarr_UrgencyLevel::Info;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresPlayerAction = false;

    FNarr_DialogueLine()
    {
        Text = TEXT("");
        Speaker = ENarr_SpeakerType::FieldResearcher;
        Type = ENarr_DialogueType::ResearchLog;
        Urgency = ENarr_UrgencyLevel::Info;
        AudioAssetPath = TEXT("");
        Duration = 5.0f;
        bRequiresPlayerAction = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    EEng_BiomeType CurrentBiome = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerFear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerHunger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float PlayerThirst = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 NearbyDinosaurs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float TimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bInDanger = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FString LastDiscovery;

    FNarr_DialogueContext()
    {
        CurrentBiome = EEng_BiomeType::Grassland;
        PlayerHealth = 100.0f;
        PlayerFear = 0.0f;
        PlayerHunger = 0.0f;
        PlayerThirst = 0.0f;
        NearbyDinosaurs = 0;
        TimeOfDay = 12.0f;
        bInDanger = false;
        LastDiscovery = TEXT("");
    }
};

/**
 * Sistema de diálogo contextual que adapta a narrativa às condições
 * de sobrevivência e descobertas do jogador no mundo pré-histórico
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerDialogue(ENarr_DialogueType Type, const FString& CustomText = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerContextualDialogue(const FNarr_DialogueContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StopCurrentDialogue();

    // ═══════════════════════════════════════════════════════════════
    // NARRATIVE TRIGGERS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerEnterDanger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnPlayerExitDanger();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnDinosaurSpotted(const FString& DinosaurType, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnResourceDiscovered(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnSurvivalStatChanged(const FString& StatName, float NewValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void OnBiomeEntered(EEng_BiomeType NewBiome);

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE DATABASE
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Database")
    FNarr_DialogueLine GetRandomDialogueForContext(const FNarr_DialogueContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Database")
    TArray<FNarr_DialogueLine> GetDialoguesForType(ENarr_DialogueType Type);

    UFUNCTION(BlueprintCallable, Category = "Database")
    void InitializeDialogueDatabase();

    // ═══════════════════════════════════════════════════════════════
    // PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_DialogueLine> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FNarr_DialogueLine CurrentDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bIsPlayingDialogue = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DialogueTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinTimeBetweenDialogues = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float LastDialogueTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableContextualDialogue = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableAudioPlayback = true;

    // ═══════════════════════════════════════════════════════════════
    // EVENTS
    // ═══════════════════════════════════════════════════════════════

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FNarr_DialogueLine&, DialogueLine);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueEnded OnDialogueEnded;

private:
    void UpdateDialogueTimer(float DeltaTime);
    void CheckForContextualTriggers();
    FNarr_DialogueLine CreateEmergencyDialogue(const FString& Message);
    FNarr_DialogueLine CreateDiscoveryDialogue(const FString& Discovery);
};