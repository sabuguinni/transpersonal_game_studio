#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "World_BiomeStreamingManager.generated.h"

// Forward declarations
class UWorld_BiomeStreamingComponent;
class UWorld_AudioProfileComponent;

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"), 
    Swamp       UMETA(DisplayName = "Swamp"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Plains      UMETA(DisplayName = "Plains"),
    Tundra      UMETA(DisplayName = "Tundra"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Coastal     UMETA(DisplayName = "Coastal")
};

UENUM(BlueprintType)
enum class EWorld_StreamingState : uint8
{
    Unloaded    UMETA(DisplayName = "Unloaded"),
    Loading     UMETA(DisplayName = "Loading"),
    Loaded      UMETA(DisplayName = "Loaded"),
    Unloading   UMETA(DisplayName = "Unloading")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeStreamingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    FVector ZoneExtents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    float StreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    float UnloadingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    FString AudioProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    EWorld_StreamingState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    TArray<TSoftObjectPtr<USoundBase>> AmbientSounds;

    FWorld_BiomeStreamingZone()
    {
        ZoneName = TEXT("DefaultZone");
        BiomeType = EWorld_BiomeType::Forest;
        ZoneCenter = FVector::ZeroVector;
        ZoneExtents = FVector(1000.0f, 1000.0f, 500.0f);
        StreamingDistance = 2000.0f;
        UnloadingDistance = 3000.0f;
        AudioProfile = TEXT("Default");
        CurrentState = EWorld_StreamingState::Unloaded;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_AudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    FString ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    TSoftObjectPtr<USoundBase> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    float AttenuationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profile")
    bool bAutoActivate;

    FWorld_AudioProfile()
    {
        ProfileName = TEXT("Default");
        Volume = 0.5f;
        AttenuationRadius = 1500.0f;
        bLooping = true;
        bAutoActivate = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeStreamingManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeStreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core streaming functionality
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void InitializeBiomeStreaming();

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UpdateStreamingBasedOnPlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void LoadBiomeZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UnloadBiomeZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    bool IsBiomeZoneLoaded(const FString& ZoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    EWorld_BiomeType GetBiomeTypeAtLocation(const FVector& Location) const;

    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void AddBiomeZone(const FWorld_BiomeStreamingZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void RemoveBiomeZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    FWorld_BiomeStreamingZone* GetBiomeZone(const FString& ZoneName);

    // Audio profile management
    UFUNCTION(BlueprintCallable, Category = "Audio Profiles")
    void RegisterAudioProfile(const FString& ProfileName, const FWorld_AudioProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Audio Profiles")
    void ActivateAudioProfile(const FString& ProfileName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Profiles")
    void DeactivateAudioProfile(const FString& ProfileName);

    // Vegetation management
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationInZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ClearVegetationInZone(const FString& ZoneName);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    float GetDistanceToNearestBiomeZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    TArray<FString> GetActiveZoneNames() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintBiomeZones();

protected:
    // Internal streaming logic
    void ProcessStreamingQueue();
    void UpdateAudioBasedOnLocation(const FVector& PlayerLocation);
    void CleanupUnusedResources();

    // Vegetation spawning helpers
    void SpawnTreesInZone(const FWorld_BiomeStreamingZone& Zone);
    void SpawnBushesInZone(const FWorld_BiomeStreamingZone& Zone);
    FVector GetRandomLocationInZone(const FWorld_BiomeStreamingZone& Zone) const;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeStreamingZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Profiles", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FWorld_AudioProfile> AudioProfiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TArray<UWorld_BiomeStreamingComponent*> StreamingComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TArray<UWorld_AudioProfileComponent*> AudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float StreamingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float MaxVegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    bool bEnableDebugVisualization;

    // Runtime state
    FVector LastPlayerLocation;
    float StreamingUpdateTimer;
    TMap<FString, TArray<AActor*>> SpawnedVegetationActors;
    TMap<FString, UAudioComponent*> ActiveAudioComponents;
};