#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsHabitable;

    FArch_StructureData()
    {
        StructureName = TEXT("Unknown Structure");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        BiomeType = EBiomeType::Savana;
        StructuralIntegrity = 100.0f;
        bIsHabitable = false;
    }
};

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Shelter         UMETA(DisplayName = "Shelter"),
    Platform        UMETA(DisplayName = "Platform"),
    Pillar          UMETA(DisplayName = "Pillar"),
    Wall            UMETA(DisplayName = "Wall"),
    Bridge          UMETA(DisplayName = "Bridge"),
    Ruins           UMETA(DisplayName = "Ruins")
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
    TArray<FArch_StructureData> RegisteredStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EBiomeType, int32> StructureCountPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxStructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatherDamageRate;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RegisterStructure(const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtBiome(EBiomeType BiomeType, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructureData> GetStructuresInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatherDamage(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetTotalStructureCount() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void PopulateAllBiomesWithStructures();

private:
    FVector GetBiomeCoordinates(EBiomeType BiomeType) const;
    void CreateBasicStructure(const FVector& Location, EArch_StructureType StructureType, EBiomeType BiomeType);
};