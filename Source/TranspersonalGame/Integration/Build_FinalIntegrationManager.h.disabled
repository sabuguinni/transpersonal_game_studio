#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Landscape/Landscape.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PlayerStart.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Build_FinalIntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Pantano     UMETA(DisplayName = "Pantano"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBuild_BiomeType BiomeType = EBuild_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 25000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<AActor*> SpawnedActors;

    FBuild_BiomeData()
    {
        BiomeType = EBuild_BiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 25000.0f;
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bIsOperational = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString LastError;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float LastValidationTime = 0.0f;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("Unknown");
        bIsOperational = false;
        LastError = TEXT("");
        LastValidationTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateGameplaySystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAssetIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemOperational(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemStatus> GetSystemStatusReport() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_BiomeData> GetBiomeStatusReport() const;

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RegisterBiomeActor(EBuild_BiomeType BiomeType, AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    int32 GetBiomeActorCount(EBuild_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FVector GetBiomeCenterLocation(EBuild_BiomeType BiomeType) const;

    // System registration
    UFUNCTION(BlueprintCallable, Category = "Systems")
    void RegisterSystemStatus(const FString& SystemName, bool bOperational, const FString& ErrorMessage = TEXT(""));

    // Diagnostic functions
    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void RunComprehensiveDiagnostics();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void LogIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void ValidateMinimumPlayableRequirements();

protected:
    // Biome data storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TMap<EBuild_BiomeType, FBuild_BiomeData> BiomeRegistry;

    // System status tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TMap<FString, FBuild_SystemStatus> SystemRegistry;

    // Integration state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bIntegrationComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float LastFullValidationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 TotalActorsInWorld = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 LoadedClassCount = 0;

private:
    // Internal validation helpers
    void ValidateWorldActors();
    void ValidateGameMode();
    void ValidatePlayerCharacter();
    void ValidateLighting();
    void ValidateNavigation();
    void ValidateDinosaurAssets();
    void ValidateEnvironmentAssets();
    
    // Biome initialization
    void InitializeBiomeRegistry();
    
    // System validation helpers
    bool ValidateClassLoading();
    bool ValidateAssetAvailability();
    bool ValidateWorldState();
};