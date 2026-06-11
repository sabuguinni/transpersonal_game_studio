#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "CretaceousArchitecturalManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_StructuralType StructuralType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasVegetationGrowth;

    FArch_StructuralElement()
    {
        ElementName = TEXT("DefaultStructure");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        StructuralType = EArch_StructuralType::Pillar;
        WeatheringLevel = 0.5f;
        bHasVegetationGrowth = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StorytellingCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString ClusterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FArch_StructuralElement> Elements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    EArch_StoryTheme StoryTheme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float DiscoveryRadius;

    FArch_StorytellingCluster()
    {
        ClusterName = TEXT("DefaultCluster");
        CenterLocation = FVector::ZeroVector;
        ClusterRadius = 1000.0f;
        StoryTheme = EArch_StoryTheme::AncientRuins;
        DiscoveryRadius = 500.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCretaceousArchitecturalManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCretaceousArchitecturalManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === STRUCTURAL PLACEMENT SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    TArray<FArch_StorytellingCluster> StorytellingClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    float MinDistanceBetweenStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    float MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Placement")
    bool bAutoGenerateStructures;

    // === WEATHERING AND AGING SYSTEM ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Weathering")
    float GlobalWeatheringRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Weathering")
    float VegetationGrowthRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Weathering")
    bool bDynamicWeathering;

    // === DISCOVERY AND INTERACTION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Discovery")
    float DiscoveryTriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Discovery")
    bool bShowDiscoveryEffects;

    // === CORE FUNCTIONALITY ===
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructuralCluster(const FVector& CenterLocation, EArch_StoryTheme Theme, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PlaceStructuralElement(const FArch_StructuralElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CheckPlayerDiscovery(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void TriggerDiscoveryEvent(const FArch_StorytellingCluster& Cluster);

    // === BIOME INTEGRATION ===
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void IntegrateWithBiome(EBiomeType BiomeType, const FVector& BiomeCenter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructuralElement> GetStructuresInRadius(const FVector& Center, float Radius);

    // === ENVIRONMENTAL STORYTELLING ===
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateStorytellingNarrative(const FString& ClusterName, EArch_StoryTheme Theme);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStorytellingElements(float DeltaTime);

private:
    // === INTERNAL STATE ===
    UPROPERTY()
    TArray<AStaticMeshActor*> SpawnedStructures;

    UPROPERTY()
    float LastWeatheringUpdate;

    UPROPERTY()
    TMap<FString, bool> DiscoveredClusters;

    // === HELPER METHODS ===
    FVector FindValidPlacementLocation(const FVector& DesiredLocation, float SearchRadius);
    bool IsLocationSuitableForStructure(const FVector& Location, EArch_StructuralType StructureType);
    void ApplyWeatheringEffects(AStaticMeshActor* Structure, float WeatheringLevel);
    void SpawnVegetationOnStructure(AStaticMeshActor* Structure);
};