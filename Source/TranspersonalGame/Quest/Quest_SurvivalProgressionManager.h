#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalProgressionManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalStage : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    WaterCollection UMETA(DisplayName = "Water Collection"),
    ShelterBuilding UMETA(DisplayName = "Shelter Building"),
    FireMaking      UMETA(DisplayName = "Fire Making"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Completed       UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class EQuest_BiomeType : uint8
{
    Pantano     UMETA(DisplayName = "Pantano"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Savana      UMETA(DisplayName = "Savana"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_BiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    FQuest_SurvivalObjective()
    {
        ObjectiveName = TEXT("");
        Description = TEXT("");
        TargetBiome = EQuest_BiomeType::Savana;
        TargetLocation = FVector::ZeroVector;
        bIsCompleted = false;
        RequiredCount = 1;
        CurrentCount = 0;
    }
};

USTRUCT(BlueprintType)
struct FQuest_BiomeQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_SurvivalObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bBiomeDiscovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    AActor* QuestIndicator;

    FQuest_BiomeQuestData()
    {
        BiomeType = EQuest_BiomeType::Savana;
        BiomeCenter = FVector::ZeroVector;
        bBiomeDiscovered = false;
        QuestIndicator = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalProgressionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalProgressionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core progression system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    EQuest_SurvivalStage CurrentStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    TArray<FQuest_BiomeQuestData> BiomeQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    TArray<FQuest_SurvivalObjective> ActiveObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    float ProgressionTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    bool bTutorialMode;

    // Visual feedback system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Feedback")
    TArray<APointLight*> QuestIndicatorLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Feedback")
    TArray<AStaticMeshActor*> TutorialWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Feedback")
    FLinearColor ActiveQuestColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Feedback")
    FLinearColor CompletedQuestColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Feedback")
    float IndicatorPulseSpeed;

    // Quest management functions
    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void InitializeBiomeQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void AdvanceToNextStage();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void CompleteObjective(const FString& ObjectiveName);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool CheckObjectiveCompletion(const FQuest_SurvivalObjective& Objective);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void UpdateObjectiveProgress(const FString& ObjectiveName, int32 ProgressAmount);

    // Visual feedback functions
    UFUNCTION(BlueprintCallable, Category = "Visual Feedback")
    void CreateQuestIndicators();

    UFUNCTION(BlueprintCallable, Category = "Visual Feedback")
    void UpdateIndicatorColors();

    UFUNCTION(BlueprintCallable, Category = "Visual Feedback")
    void CreateTutorialWaypoints();

    UFUNCTION(BlueprintCallable, Category = "Visual Feedback")
    void ShowObjectiveMarker(const FVector& Location, const FString& ObjectiveName);

    // Biome interaction
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void OnPlayerEnterBiome(EQuest_BiomeType BiomeType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FQuest_BiomeQuestData* GetBiomeQuestData(EQuest_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FQuest_SurvivalObjective> GetObjectivesForBiome(EQuest_BiomeType BiomeType);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Quest Utils")
    FVector GetRandomLocationInBiome(EQuest_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Quest Utils")
    float GetDistanceToNearestObjective(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Utils")
    FString GetCurrentStageDescription();

    UFUNCTION(BlueprintCallable, Category = "Quest Utils")
    float GetOverallProgressPercentage();

private:
    // Internal helper functions
    void SetupBiomeData();
    void CreateObjectivesForBiome(EQuest_BiomeType BiomeType);
    void UpdateVisualFeedback(float DeltaTime);
    FVector GetBiomeCenterLocation(EQuest_BiomeType BiomeType);
    void PulseIndicatorLights(float DeltaTime);
};