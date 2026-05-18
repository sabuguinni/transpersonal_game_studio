#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    Pillar          UMETA(DisplayName = "Stone Pillar"),
    Arch            UMETA(DisplayName = "Stone Arch"),
    Ruin            UMETA(DisplayName = "Ancient Ruin"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    StonePlatform   UMETA(DisplayName = "Stone Platform"),
    Megalith        UMETA(DisplayName = "Megalith")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMoss = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsRuin = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savana;
        WeatheringLevel = 0.5f;
        bHasMoss = true;
        bIsRuin = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArchitecturalStructureManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> StructureDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EBiomeType, int32> StructureCountPerBiome;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<AActor*> SpawnedStructures;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringToStructure(AActor* Structure, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetBiomeCenter(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForStructure(FVector Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateStructuresForAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ClearAllStructures();

private:
    FArch_StructureData CreateStructureData(EArch_StructureType Type, FVector Location, EBiomeType Biome);
    AActor* SpawnStructureMesh(const FArch_StructureData& StructureData);
    void InitializeBiomeStructureCounts();
};