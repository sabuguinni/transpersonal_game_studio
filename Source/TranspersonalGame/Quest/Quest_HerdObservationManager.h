#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Quest_HerdObservationManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ObservationType : uint8
{
    None UMETA(DisplayName = "None"),
    CountAnimals UMETA(DisplayName = "Count Animals"),
    WatchFeeding UMETA(DisplayName = "Watch Feeding"),
    TrackMovement UMETA(DisplayName = "Track Movement"),
    StudyBehavior UMETA(DisplayName = "Study Behavior"),
    AvoidPredator UMETA(DisplayName = "Avoid Predator")
};

UENUM(BlueprintType)
enum class EQuest_BiomeType : uint8
{
    Savana UMETA(DisplayName = "Savana"),
    Pantano UMETA(DisplayName = "Pantano"),
    Floresta UMETA(DisplayName = "Floresta"),
    Deserto UMETA(DisplayName = "Deserto"),
    Montanha UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObservationTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObservationType ObservationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_BiomeType TargetBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ObservationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ProgressPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_ObservationTask()
    {
        TaskName = TEXT("Unknown Task");
        ObservationType = EQuest_ObservationType::None;
        TargetBiome = EQuest_BiomeType::Savana;
        TargetSpecies = TEXT("Unknown");
        RequiredCount = 1;
        ObservationTime = 30.0f;
        ProgressPercent = 0.0f;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_BiomeLocation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float Radius;

    FQuest_BiomeLocation()
    {
        BiomeType = EQuest_BiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 15000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_HerdObservationManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_HerdObservationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_ObservationTask> ActiveTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_BiomeLocation> BiomeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float PlayerDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float TaskUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    AActor* PlayerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 CompletedTaskCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float TotalObservationTime;

private:
    FTimerHandle TaskUpdateTimer;
    float LastUpdateTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeBiomeLocations();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateObservationTask(const FString& TaskName, EQuest_ObservationType ObsType, 
                              EQuest_BiomeType TargetBiome, const FString& Species, 
                              int32 Count, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateTaskProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsPlayerInBiome(EQuest_BiomeType BiomeType, FVector& OutBiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<AActor*> FindDinosaursInRange(const FVector& Location, float Range, const FString& SpeciesFilter);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteTask(int32 TaskIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FString GetTaskStatusString(int32 TaskIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartBasicObservationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartAdvancedTrackingQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartSurvivalObservationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetActiveTaskCount() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    float GetQuestCompletionPercent() const;
};