#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "Quest_BiomeExplorationSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_BiomeType : uint8
{
    Pantano     UMETA(DisplayName = "Pântano"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Savana      UMETA(DisplayName = "Savana"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha Nevada")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_BiomeExplorationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quest")
    EQuest_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quest")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quest")
    FString ExplorationObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quest")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quest")
    float ExplorationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quest")
    bool bIsExplored;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Quest")
    float ExplorationProgress;

    FQuest_BiomeExplorationData()
    {
        BiomeType = EQuest_BiomeType::Savana;
        BiomeName = TEXT("Savana Central");
        ExplorationObjective = TEXT("Explorar área desconhecida");
        CenterLocation = FVector::ZeroVector;
        ExplorationRadius = 5000.0f;
        bIsExplored = false;
        ExplorationProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ExplorationReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    TArray<FString> UnlockedAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    TArray<FString> DiscoveredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    FString NewKnowledge;

    FQuest_ExplorationReward()
    {
        ExperiencePoints = 100;
        NewKnowledge = TEXT("Conhecimento sobre o bioma");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_BiomeExplorationTrigger : public ATriggerVolume
{
    GENERATED_BODY()

public:
    AQuest_BiomeExplorationTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Exploration")
    FQuest_BiomeExplorationData BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Exploration")
    FQuest_ExplorationReward ExplorationReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Exploration")
    float TimeRequiredForExploration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Exploration")
    bool bRequiresSpecificActions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Exploration")
    TArray<FString> RequiredActions;

private:
    UPROPERTY()
    float CurrentExplorationTime;

    UPROPERTY()
    bool bPlayerInArea;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Exploration")
    void StartExploration();

    UFUNCTION(BlueprintCallable, Category = "Biome Exploration")
    void CompleteExploration();

    UFUNCTION(BlueprintCallable, Category = "Biome Exploration")
    float GetExplorationProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Exploration")
    bool IsExplorationComplete() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Biome Exploration")
    void OnExplorationStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Biome Exploration")
    void OnExplorationCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Biome Exploration")
    void OnExplorationProgressUpdated(float Progress);

protected:
    UFUNCTION()
    void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

public:
    virtual void Tick(float DeltaTime) override;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_BiomeExplorationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_BiomeExplorationComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration System")
    TArray<FQuest_BiomeExplorationData> AllBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration System")
    TMap<EQuest_BiomeType, bool> BiomeDiscoveryStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration System")
    float TotalExplorationProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration System")
    int32 TotalExperienceGained;

public:
    UFUNCTION(BlueprintCallable, Category = "Exploration System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Exploration System")
    void RegisterBiomeExploration(EQuest_BiomeType BiomeType, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Exploration System")
    void CompleteBiomeExploration(EQuest_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Exploration System")
    float GetOverallExplorationProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Exploration System")
    TArray<EQuest_BiomeType> GetUnexploredBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Exploration System")
    bool IsBiomeExplored(EQuest_BiomeType BiomeType) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Exploration System")
    void OnBiomeDiscovered(EQuest_BiomeType BiomeType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Exploration System")
    void OnAllBiomesExplored();

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};