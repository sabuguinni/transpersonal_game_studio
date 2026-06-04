#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Narr_TribalDialogueSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_TribalRole : uint8
{
    Elder           UMETA(DisplayName = "Tribal Elder"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Scout           UMETA(DisplayName = "Scout"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Child           UMETA(DisplayName = "Child")
};

UENUM(BlueprintType)
enum class ENarr_SurvivalContext : uint8
{
    Safe            UMETA(DisplayName = "Safe Area"),
    Dangerous       UMETA(DisplayName = "Dangerous Territory"),
    Hunting         UMETA(DisplayName = "Hunting Grounds"),
    Gathering       UMETA(DisplayName = "Resource Area"),
    Settlement      UMETA(DisplayName = "Tribal Settlement"),
    Unknown         UMETA(DisplayName = "Unknown Territory"),
    Sacred          UMETA(DisplayName = "Sacred Site"),
    Forbidden       UMETA(DisplayName = "Forbidden Zone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_TribalDialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_TribalRole RequiredRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_SurvivalContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    int32 RelationshipThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float ThreatLevel;

    FNarr_TribalDialogueEntry()
    {
        DialogueText = TEXT("");
        RequiredRole = ENarr_TribalRole::Elder;
        Context = ENarr_SurvivalContext::Settlement;
        RelationshipThreshold = 0;
        ThreatLevel = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_TribalKnowledge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    FString KnowledgeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    int32 RequiredTrust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    TArray<FString> Prerequisites;

    FNarr_TribalKnowledge()
    {
        KnowledgeType = TEXT("");
        Description = TEXT("");
        RequiredTrust = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_TribalDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_TribalDialogueComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    ENarr_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString TribalName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    int32 PlayerTrust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    int32 PlayerRespect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float CurrentFear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_TribalDialogueEntry> AvailableDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    TArray<FNarr_TribalKnowledge> TribalKnowledge;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetContextualDialogue(ENarr_SurvivalContext Context, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FString> GetPlayerResponses(const FString& DialogueKey);

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    void ModifyTrust(int32 TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    void ModifyRespect(int32 RespectChange);

    UFUNCTION(BlueprintCallable, Category = "Knowledge")
    bool CanShareKnowledge(const FString& KnowledgeType);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalState(float Fear, float Hunger);

protected:
    virtual void BeginPlay() override;

private:
    void InitializeTribalDialogue();
    void InitializeTribalKnowledge();
    FString SelectDialogueByRole(ENarr_SurvivalContext Context);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_TribalDialogueManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_TribalDialogueManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue System")
    TMap<ENarr_TribalRole, TArray<FNarr_TribalDialogueEntry>> RoleBasedDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    ENarr_SurvivalContext CurrentContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Context")
    float AreaThreatLevel;

    UFUNCTION(BlueprintCallable, Category = "Dialogue Management")
    void InitializeTribalDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue Management")
    FString ProcessTribalInteraction(AActor* NPC, AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Context")
    void UpdateSurvivalContext(ENarr_SurvivalContext NewContext, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Relationships")
    void UpdateTribalRelationships(AActor* NPC, int32 TrustChange, int32 RespectChange);

protected:
    virtual void BeginPlay() override;

private:
    void LoadTribalDialogueDatabase();
    void SetupContextualResponses();
};