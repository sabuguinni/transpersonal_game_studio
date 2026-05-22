#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    AncientRuin     UMETA(DisplayName = "Ancient Ruin")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::StonePillar;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> ManagedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructureSpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoGenerateStructures;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void GenerateStructuresForBiome(EBiomeType BiomeType, int32 StructureCount = 5);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringToStructure(AActor* StructureActor, float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetAllManagedStructures() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

private:
    UPROPERTY()
    TArray<AActor*> SpawnedStructureActors;

    void InitializeStructureSystem();
    AActor* CreateStructureActor(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation);
    void SetupStructureAppearance(AActor* StructureActor, const FArch_StructureData& StructureData);
};