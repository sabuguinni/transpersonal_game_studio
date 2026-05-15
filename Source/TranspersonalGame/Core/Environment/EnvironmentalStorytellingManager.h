#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvironmentalStorytellingManager.generated.h"

USTRUCT(BlueprintType)
struct FEnvArt_PropCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FVector> PropLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FRotator> PropRotations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FString> PropTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString StoryTheme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float ClusterRadius;

    FEnvArt_PropCluster()
    {
        StoryTheme = TEXT("Abandoned Campsite");
        ClusterRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float LightIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    EBiomeType BiomeType;

    FEnvArt_AtmosphericZone()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 2000.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        LightIntensityMultiplier = 1.0f;
        BiomeType = EBiomeType::Forest;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentalStorytellingManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentalStorytellingManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FEnvArt_PropCluster> StorytellingClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FEnvArt_AtmosphericZone> AtmosphericZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float PropSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    int32 MaxPropsPerCluster;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateStorytellingCluster(const FVector& Location, const FString& Theme, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void SpawnAbandonedCampsite(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void SpawnDinosaurRemains(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void SpawnPrimitiveTools(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericZone(const FVector& Center, float Radius, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateZoneAtmosphere(const FEnvArt_AtmosphericZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateBiomeWithProps(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Environment", CallInEditor)
    void GenerateEnvironmentalStorytellingForAllBiomes();

private:
    UFUNCTION()
    void SpawnPropAtLocation(const FVector& Location, const FRotator& Rotation, const FString& PropType);

    UFUNCTION()
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);

    UFUNCTION()
    bool IsLocationSuitableForProp(const FVector& Location);

    TArray<AActor*> SpawnedProps;
    
    float LastAtmosphereUpdateTime;
    static constexpr float AtmosphereUpdateInterval = 5.0f;
};