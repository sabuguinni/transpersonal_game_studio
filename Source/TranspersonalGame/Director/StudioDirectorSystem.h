#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float CompletionPercentage;

    FDir_AgentTask()
    {
        AgentID = TEXT("");
        TaskDescription = TEXT("");
        Priority = 0;
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated;

    FDir_BiomeStatus()
    {
        BiomeName = TEXT("");
        BiomeLocation = FVector::ZeroVector;
        ActorCount = 0;
        bIsPopulated = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorSystem : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    bool bPlayablePrototypeReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Studio Director")
    float OverallProgress;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentTask(const FString& AgentID, float Progress, bool bCompleted = false);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CheckBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float CalculateOverallProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetPendingTasks() const;

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsPlayablePrototypeReady() const { return bPlayablePrototypeReady; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Studio Director")
    void DebugPrintStatus();

private:
    void InitializeBiomeStatuses();
    bool ValidateCharacterMovement();
    bool ValidateDinosaurPresence();
    bool ValidateTerrainAndLighting();
};