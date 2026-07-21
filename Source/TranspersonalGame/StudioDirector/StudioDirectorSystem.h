#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Working UMETA(DisplayName = "Working"), 
    Completed UMETA(DisplayName = "Completed"),
    Failed UMETA(DisplayName = "Failed"),
    Timeout UMETA(DisplayName = "Timeout")
};

UENUM(BlueprintType)
enum class EDir_BiomeType : uint8
{
    Savana UMETA(DisplayName = "Savana"),
    Pantano UMETA(DisplayName = "Pantano"),
    Floresta UMETA(DisplayName = "Floresta"),
    Deserto UMETA(DisplayName = "Deserto"),
    Montanha UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct FDir_AgentTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 AgentNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString TaskDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    EDir_AgentStatus Status = EDir_AgentStatus::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    int32 Priority = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    float EstimatedDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent Task")
    TArray<FString> Deliverables;

    FDir_AgentTask()
    {
        AgentNumber = 0;
        AgentName = TEXT("");
        TaskDescription = TEXT("");
        Status = EDir_AgentStatus::Idle;
        Priority = 1;
        EstimatedDuration = 0.0f;
        CycleID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FDir_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EDir_BiomeType BiomeType = EDir_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsPopulated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> SpawnedActors;

    FDir_BiomeStatus()
    {
        BiomeType = EDir_BiomeType::Savana;
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        DinosaurCount = 0;
        bIsPopulated = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTask(int32 AgentNumber, const FString& TaskDescription, int32 Priority = 1);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentTask> GetActiveAgentTasks();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentTask GetAgentTask(int32 AgentNumber);

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateBiomeStatus(EDir_BiomeType BiomeType, int32 ActorCount, int32 DinosaurCount);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_BiomeStatus> GetAllBiomeStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool IsBiomePopulated(EDir_BiomeType BiomeType);

    // Prototype Validation
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    bool ValidateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FString> GetPrototypeRequirements();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetPrototypeCompletionPercentage();

    // Cycle Management
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void StartNewCycle(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteCycle();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GetCurrentCycleID() const { return CurrentCycleID; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_AgentTask> AgentTasks;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    TArray<FDir_BiomeStatus> BiomeStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    FString CurrentCycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    float CycleStartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Studio Director")
    bool bPrototypeComplete = false;

private:
    void InitializeAgentTasks();
    void InitializeBiomeStatuses();
    void ValidateAgentDependencies();
    void CheckPrototypeRequirements();
};