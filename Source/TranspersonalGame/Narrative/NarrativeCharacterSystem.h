#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "NarrativeCharacterSystem.generated.h"

// Character personality traits for narrative interactions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Courage = 50.0f; // 0-100, affects danger responses

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Wisdom = 50.0f; // 0-100, affects knowledge sharing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggression = 50.0f; // 0-100, affects combat readiness

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Trust = 50.0f; // 0-100, affects player relationship

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Curiosity = 50.0f; // 0-100, affects exploration behavior
};

// NPC relationship status with player
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float TrustLevel = 0.0f; // -100 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float RespectLevel = 0.0f; // -100 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    int32 InteractionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    bool bHasMetPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    FDateTime LastInteraction;
};

// Character background and story elements
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterBackground
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    ENPCRole Role = ENPCRole::Survivor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FString OriginTribe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    TArray<FString> PersonalHistory; // Key life events

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    TArray<FString> Skills; // Hunting, crafting, tracking, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FString CurrentGoal; // What they're trying to achieve

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FNarr_CharacterPersonality Personality;
};

// Component for NPCs to handle narrative interactions
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeCharacterComponent();

protected:
    virtual void BeginPlay() override;

public:
    // Character data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Character")
    FNarr_CharacterBackground Background;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Character")
    FNarr_NPCRelationship PlayerRelationship;

    // Dialogue state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CurrentMood; // Affects dialogue choices

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanInteract = true;

    // Story progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedStoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString CurrentStoryArc;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateRelationship(float TrustChange, float RespectChange);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetContextualDialogue(const FString& Context);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanDiscussStoryBeat(const FString& StoryBeat) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkStoryBeatCompleted(const FString& StoryBeat);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    float GetPersonalityTrait(const FString& TraitName) const;
};

// Subsystem to manage all narrative characters and their relationships
UCLASS()
class TRANSPERSONALGAME_API UNarrativeCharacterSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    // All registered narrative characters
    UPROPERTY()
    TMap<FString, UNarrativeCharacterComponent*> RegisteredCharacters;

    // Global relationship tracking
    UPROPERTY()
    TMap<FString, FNarr_NPCRelationship> PlayerRelationships;

    // Story progression tracking
    UPROPERTY()
    TArray<FString> GlobalStoryBeats;

    UPROPERTY()
    FString CurrentMainStoryArc;

public:
    // Character management
    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    void RegisterCharacter(const FString& CharacterID, UNarrativeCharacterComponent* Character);

    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    UNarrativeCharacterComponent* GetCharacter(const FString& CharacterID);

    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    TArray<UNarrativeCharacterComponent*> GetCharactersByRole(ENPCRole Role);

    // Relationship management
    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    void UpdateGlobalRelationship(const FString& CharacterID, float TrustChange, float RespectChange);

    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    float GetRelationshipLevel(const FString& CharacterID, const FString& RelationType);

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    void AdvanceStoryBeat(const FString& StoryBeat);

    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    bool IsStoryBeatCompleted(const FString& StoryBeat) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    void SetMainStoryArc(const FString& NewArc);

    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    FString GetCurrentStoryArc() const { return CurrentMainStoryArc; }

    // Contextual narrative
    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    TArray<FString> GetRelevantCharactersForLocation(const FVector& Location, float Radius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Narrative System")
    FString GenerateContextualNarration(const FString& Context, const FVector& Location);
};