#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Pillar         UMETA(DisplayName = "Stone Pillar"),
    Wall          UMETA(DisplayName = "Stone Wall"),
    Foundation    UMETA(DisplayName = "Foundation"),
    Archway       UMETA(DisplayName = "Archway"),
    Platform      UMETA(DisplayName = "Platform"),
    Ruins         UMETA(DisplayName = "Ancient Ruins")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Limestone     UMETA(DisplayName = "Limestone"),
    Sandstone     UMETA(DisplayName = "Sandstone"),
    Basalt        UMETA(DisplayName = "Basalt"),
    Granite       UMETA(DisplayName = "Granite"),
    Clay          UMETA(DisplayName = "Clay Brick"),
    Wood          UMETA(DisplayName = "Petrified Wood")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Pillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType MaterialType = EArch_MaterialType::Limestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(100.0f, 100.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMoss = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsRuined = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString HistoricalContext = TEXT("Ancient Cretaceous structure");

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Pillar;
        MaterialType = EArch_MaterialType::Limestone;
        Dimensions = FVector(100.0f, 100.0f, 300.0f);
        WeatheringLevel = 0.5f;
        bHasMoss = true;
        bIsRuined = false;
        HistoricalContext = TEXT("Ancient Cretaceous structure");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructure : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructure();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    UStaticMesh* PillarMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    UStaticMesh* WallMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    UStaticMesh* FoundationMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetMaterialType(EArch_MaterialType NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetMossGrowth(bool bEnableMoss);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_StructureData GetStructureData() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void RegenerateStructure();

private:
    void UpdateStructureMesh();
    void ApplyMaterialProperties();
    void SetupCollision();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArchitecturalStructureManager : public UObject
{
    GENERATED_BODY()

public:
    UArchitecturalStructureManager();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    static UArchitecturalStructureManager* GetInstance();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AArchitecturalStructure* SpawnStructure(UWorld* World, const FVector& Location, EArch_StructureType StructureType, EArch_MaterialType MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AArchitecturalStructure*> SpawnStructureCluster(UWorld* World, const FVector& CenterLocation, int32 Count, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PlaceStructuresInBiome(UWorld* World, EBiomeType BiomeType, int32 StructureCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AArchitecturalStructure*> GetAllStructures(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyBiomeWeathering(UWorld* World, EBiomeType BiomeType);

private:
    static UArchitecturalStructureManager* Instance;

    UPROPERTY()
    TArray<TWeakObjectPtr<AArchitecturalStructure>> ManagedStructures;

    FVector GetBiomeCoordinates(EBiomeType BiomeType);
    EArch_MaterialType GetBiomeMaterial(EBiomeType BiomeType);
    float GetBiomeWeathering(EBiomeType BiomeType);
};