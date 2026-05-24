#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../Crowd/CrowdSimulationManager.h"
#include "../Core/TranspersonalGameState.h"
#include "Quest_MigrationTracker.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MigrationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FString HerdSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsActive;

    FQuest_MigrationData()
    {
        HerdSpecies = TEXT("Unknown");
        HerdSize = 0;
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector::ZeroVector;
        MigrationProgress = 0.0f;
        bIsActive = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API AQuest_MigrationTracker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MigrationTracker();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    class USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    class UStaticMeshComponent* QuestMarker;

    // Migration Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    TArray<FQuest_MigrationData> TrackedMigrations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    int32 RequiredMigrationsToTrack;

    // Quest State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    bool bQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    int32 MigrationsTracked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    float QuestTimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    float QuestDuration;

    // Crowd System Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    class UCrowdSimulationManager* CrowdManager;

    // Quest Functions
    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    void StartMigrationQuest();

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    void CompleteMigrationQuest();

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    void UpdateMigrationTracking();

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    bool DetectNearbyHerds();

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    void AddMigrationToTracker(const FString& Species, int32 HerdSize, FVector StartPos, FVector EndPos);

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    float CalculateMigrationProgress(const FQuest_MigrationData& Migration);

    // Player Interaction
    UFUNCTION()
    void OnPlayerEnterDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitDetection(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    float LastUpdateTime;
    bool bPlayerInRange;
    
    void UpdateQuestMarkerVisibility();
    void ProcessMigrationRewards();
    void LogMigrationProgress();
};