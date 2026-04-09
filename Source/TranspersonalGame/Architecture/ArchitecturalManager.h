#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "ArchitecturalManager.generated.h"

UENUM(BlueprintType)
enum class EArchitecturalStyle : uint8
{
    PrehistoricCave      UMETA(DisplayName = "Prehistoric Cave"),
    PrimitiveHut         UMETA(DisplayName = "Primitive Hut"),
    StoneCircle          UMETA(DisplayName = "Stone Circle"),
    AncientRuins         UMETA(DisplayName = "Ancient Ruins"),
    TribalShelter        UMETA(DisplayName = "Tribal Shelter"),
    SacredGrove          UMETA(DisplayName = "Sacred Grove"),
    BurialMound          UMETA(DisplayName = "Burial Mound")
};

UENUM(BlueprintType)
enum class EBuildingPurpose : uint8
{
    Shelter              UMETA(DisplayName = "Shelter"),
    Storage              UMETA(DisplayName = "Storage"),
    Ritual               UMETA(DisplayName = "Ritual"),
    Defense              UMETA(DisplayName = "Defense"),
    Crafting             UMETA(DisplayName = "Crafting"),
    Burial               UMETA(DisplayName = "Burial"),
    Gathering            UMETA(DisplayName = "Gathering")
};

USTRUCT(BlueprintType)
struct FArchitecturalSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArchitecturalStyle Style;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingPurpose Purpose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Dimensions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DeteriorationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasInterior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> InteriorStoryElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AgeInYears;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAbandoned;

    FArchitecturalSpec()
    {
        Style = EArchitecturalStyle::PrimitiveHut;
        Purpose = EBuildingPurpose::Shelter;
        Dimensions = FVector(500.0f, 500.0f, 300.0f);
        DeteriorationLevel = 0.3f;
        bHasInterior = true;
        AgeInYears = 100;
        bIsAbandoned = false;
    }
};

/**
 * Architectural Manager - Creates and manages all built structures in the prehistoric world
 * Each building tells a story through its design, deterioration, and interior details
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    class UPCGComponent* PCGComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    class UPCGGraphInterface* BuildingGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArchitecturalSpec> BuildingSpecs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EArchitecturalStyle, class UStaticMesh*> StyleMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EBuildingPurpose, class UMaterialInterface*> PurposeMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenBuildings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxBuildingsPerSquareKm;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateBuildingsInArea(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* CreateBuilding(const FArchitecturalSpec& Spec, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyDeteriorationToBuilding(AActor* Building, float DeteriorationLevel);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateInterior(AActor* Building, const FArchitecturalSpec& Spec);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FVector> FindSuitableBuildingLocations(const FVector& Center, float Radius, int32 MaxBuildings);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForBuilding(const FVector& Location, EArchitecturalStyle Style);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateEnvironmentalStorytelling(AActor* Building, const FArchitecturalSpec& Spec);

private:
    void InitializeBuildingAssets();
    void SetupPCGGeneration();
    FArchitecturalSpec GenerateRandomBuildingSpec();
    void ApplyWeatheringEffects(AActor* Building, float Age, float Deterioration);
    void AddInteriorDetails(AActor* Building, const TArray<FString>& StoryElements);
};