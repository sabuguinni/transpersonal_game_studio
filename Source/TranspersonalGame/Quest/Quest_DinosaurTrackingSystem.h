#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_DinosaurTrackingSystem.generated.h"

USTRUCT(BlueprintType)
struct FQuest_TrackingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
    FString DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
    float TrackingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
    float TimeLastSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
    bool bIsActiveTarget;

    FQuest_TrackingData()
    {
        DinosaurSpecies = TEXT("Unknown");
        LastKnownLocation = FVector::ZeroVector;
        TrackingRadius = 2000.0f;
        TimeLastSeen = 0.0f;
        bIsActiveTarget = false;
    }
};

UENUM(BlueprintType)
enum class EQuest_TrackingObjective : uint8
{
    FindTracks,
    FollowTracks,
    ObserveDinosaur,
    StudyBehavior,
    CountHerd,
    IdentifySpecies
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_DinosaurTrackingSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_DinosaurTrackingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core tracking components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TrackingZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* TrackingMarker;

    // Tracking data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    TArray<FQuest_TrackingData> ActiveTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    EQuest_TrackingObjective CurrentObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    FString TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    int32 RequiredObservations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    int32 CompletedObservations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    float TrackingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Tracking")
    bool bQuestActive;

    // Biome-specific tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Tracking")
    TMap<FString, FVector> BiomeTrackingLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Tracking")
    FString CurrentBiome;

    // Quest functions
    UFUNCTION(BlueprintCallable, Category = "Quest Tracking")
    void StartTrackingQuest(const FString& Species, EQuest_TrackingObjective Objective, int32 RequiredCount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest Tracking")
    void AddTrackingData(const FString& Species, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Tracking")
    bool CheckTrackingProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest Tracking")
    void CompleteTrackingObjective();

    UFUNCTION(BlueprintCallable, Category = "Quest Tracking")
    FVector GetNearestTrackLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Tracking")
    TArray<FString> GetAvailableSpeciesInBiome(const FString& BiomeName);

    // Biome detection
    UFUNCTION(BlueprintCallable, Category = "Biome Detection")
    FString DetectCurrentBiome(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Detection")
    void InitializeBiomeTrackingPoints();

    // Overlap detection
    UFUNCTION()
    void OnTrackingZoneOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTrackingZoneOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    void UpdateTrackingMarkers();
    void ScanForDinosaurs();
    void ProcessTrackingObjective();
    bool IsValidDinosaurActor(AActor* Actor);
    FString GetDinosaurSpeciesFromActor(AActor* Actor);
};