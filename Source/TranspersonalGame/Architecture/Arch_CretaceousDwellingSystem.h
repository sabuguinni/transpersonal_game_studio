#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_CretaceousDwellingSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    StoneSheller    UMETA(DisplayName = "Stone Shelter"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang"),
    UndergroundCave UMETA(DisplayName = "Underground Cave"),
    CliffDwelling   UMETA(DisplayName = "Cliff Dwelling")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DwellingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingType DwellingType = EArch_DwellingType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float InteriorLightIntensity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    FLinearColor InteriorLightColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasToolScatter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float StructuralIntegrity = 100.0f;

    FArch_DwellingConfig()
    {
        DwellingType = EArch_DwellingType::CaveEntrance;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        InteriorLightIntensity = 500.0f;
        InteriorLightColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
        bHasFirePit = true;
        bHasToolScatter = true;
        StructuralIntegrity = 100.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_CretaceousDwellingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_CretaceousDwellingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core dwelling creation methods
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    AActor* CreateDwelling(const FArch_DwellingConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void SpawnCaveEntrance(FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void SpawnStoneSheller(FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void SpawnRockOverhang(FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    // Interior setup methods
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetupInteriorLighting(AActor* DwellingActor, const FArch_DwellingConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PlaceFirePit(AActor* DwellingActor, FVector RelativeLocation = FVector(100, 0, -50));

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ScatterPrimitiveTools(AActor* DwellingActor, int32 ToolCount = 5);

    // Biome-specific spawning
    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void SpawnDwellingsInBiome(EBiomeType BiomeType, int32 DwellingCount = 3);

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetAllDwellings() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetDwellingIntegrity(AActor* DwellingActor) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairDwelling(AActor* DwellingActor, float RepairAmount = 25.0f);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_DwellingConfig> DefaultDwellingConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    UMaterialInterface* CaveMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float DwellingSpawnRadius = 50000.0f;

    // Runtime tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<AActor*> SpawnedDwellings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    int32 TotalDwellingsCreated = 0;

private:
    // Internal helper methods
    AActor* CreateBasicDwellingStructure(const FArch_DwellingConfig& Config);
    void ApplyDwellingMaterials(AActor* DwellingActor, EArch_DwellingType DwellingType);
    FVector CalculateBiomeSpawnLocation(EBiomeType BiomeType, int32 Index) const;
    void ValidateDwellingPlacement(FVector& Location, FRotator& Rotation) const;
};