#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "World_MassiveBiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_MassiveBiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Biome")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Biome")
    float Size;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Biome")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Biome")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive Biome")
    bool bIsPopulated;

    FWorld_MassiveBiomeZone()
    {
        Center = FVector::ZeroVector;
        Size = 100000.0f;
        BiomeType = EWorld_BiomeType::Savanna;
        Description = TEXT("Default Biome Zone");
        ActorCount = 0;
        bIsPopulated = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TArray<AActor*> ManagedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsActive;

    FWorld_StreamingZone()
    {
        ZoneName = TEXT("DefaultZone");
        Center = FVector::ZeroVector;
        StreamingDistance = 300000.0f;
        bIsActive = false;
    }
};

/**
 * Manages massive 200km² biome zones with streaming and population control
 * Handles the expanded world coordinates and biome distribution
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MassiveBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MassiveBiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === MASSIVE WORLD CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive World", meta = (AllowPrivateAccess = "true"))
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive World", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_MassiveBiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive World", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_StreamingZone> StreamingZones;

    // === BIOME MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Massive Biome")
    void InitializeMassiveBiomes();

    UFUNCTION(BlueprintCallable, Category = "Massive Biome")
    void PopulateBiomeZone(EWorld_BiomeType BiomeType, int32 MaxActors = 100);

    UFUNCTION(BlueprintCallable, Category = "Massive Biome")
    FWorld_MassiveBiomeZone GetBiomeZoneByType(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Massive Biome")
    EWorld_BiomeType GetBiomeTypeAtLocation(const FVector& Location);

    // === STREAMING MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateStreamingZones(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void ActivateStreamingZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void DeactivateStreamingZone(const FString& ZoneName);

    // === PERFORMANCE OPTIMIZATION ===

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomeActors(float DistanceFromPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetTotalManagedActors() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupDuplicateActors();

protected:
    // === INTERNAL SYSTEMS ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float StreamingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bEnableStreamingOptimization;

private:
    // === INTERNAL TRACKING ===

    float LastStreamingUpdate;
    int32 TotalSpawnedActors;
    bool bMassiveBiomesInitialized;

    // === HELPER FUNCTIONS ===

    void CreateBiomeMarkers();
    void CreateStreamingTriggers();
    FVector GetRandomLocationInBiome(const FWorld_MassiveBiomeZone& BiomeZone);
    void SpawnBiomeActors(const FWorld_MassiveBiomeZone& BiomeZone, int32 Count);
};