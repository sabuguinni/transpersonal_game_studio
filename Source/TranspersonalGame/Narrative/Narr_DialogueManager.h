#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

// Estrutura para uma linha de diálogo
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsNarration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    FNarr_DialogueLine()
    {
        SpeakerName = "Unknown";
        DialogueText = "";
        Duration = 3.0f;
        bIsNarration = false;
        AudioPath = "";
    }
};

// Estrutura para uma conversa completa
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_Conversation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ConversationID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEng_QuestType TriggerContext;

    FNarr_Conversation()
    {
        ConversationID = "DefaultConversation";
        bIsRepeatable = true;
        TriggerContext = EEng_QuestType::Survival;
    }
};

// Enum para tipos de eventos narrativos
UENUM(BlueprintType)
enum class ENarr_EventType : uint8
{
    Discovery       UMETA(DisplayName = "Discovery"),
    Danger          UMETA(DisplayName = "Danger"),
    Weather         UMETA(DisplayName = "Weather"),
    DinosaurSight   UMETA(DisplayName = "Dinosaur Sighting"),
    ResourceFound   UMETA(DisplayName = "Resource Found"),
    QuestUpdate     UMETA(DisplayName = "Quest Update"),
    Environmental   UMETA(DisplayName = "Environmental")
};

// Delegate para eventos de diálogo
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueEvent, FString, DialogueText, FString, SpeakerName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeEvent, ENarr_EventType, EventType);

/**
 * Gestor de diálogos e narração para o jogo de sobrevivência
 * Gere conversas, narração contextual e eventos narrativos
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PROPRIEDADES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TArray<FNarr_Conversation> AvailableConversations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    FNarr_Conversation CurrentConversation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    int32 CurrentLineIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    bool bIsDialogueActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    float DialogueDisplayTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Events")
    TMap<ENarr_EventType, TArray<FString>> ContextualNarrations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Events")
    float LastNarrationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Events")
    float MinNarrationInterval;

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEvent OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEvent OnDialogueLineChanged;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue Events")
    FOnDialogueEvent OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnNarrativeEvent OnNarrativeEventTriggered;

    // === MÉTODOS PÚBLICOS ===

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartConversation(const FString& ConversationID);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void NextDialogueLine();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndConversation();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_DialogueLine GetCurrentDialogueLine() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarrativeEvent(ENarr_EventType EventType, const FString& CustomText = "");

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddContextualNarration(ENarr_EventType EventType, const FString& NarrationText);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RegisterConversation(const FNarr_Conversation& NewConversation);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FNarr_Conversation FindConversationByID(const FString& ConversationID) const;

    // === MÉTODOS DE NARRAÇÃO CONTEXTUAL ===

    UFUNCTION(BlueprintCallable, Category = "Contextual Narrative")
    void TriggerDiscoveryNarration(const FString& DiscoveryType);

    UFUNCTION(BlueprintCallable, Category = "Contextual Narrative")
    void TriggerDangerNarration(EEng_DinosaurSpecies DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Contextual Narrative")
    void TriggerWeatherNarration(EEng_WeatherType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Contextual Narrative")
    void TriggerBiomeNarration(EEng_BiomeType BiomeType);

protected:
    // === MÉTODOS PROTEGIDOS ===

    UFUNCTION()
    void InitializeDefaultConversations();

    UFUNCTION()
    void InitializeContextualNarrations();

    UFUNCTION()
    void UpdateDialogueTimer(float DeltaTime);

    UFUNCTION()
    FString GetRandomNarrationForEvent(ENarr_EventType EventType) const;

    UFUNCTION()
    bool CanTriggerNarration() const;

private:
    // Timer para controlo de diálogo
    float DialogueTimer;
    
    // Histórico de narrações para evitar repetições
    TArray<FString> RecentNarrations;
    int32 MaxRecentNarrations;
};