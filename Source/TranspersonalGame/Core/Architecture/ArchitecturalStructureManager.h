#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None        UMETA(DisplayName = "None"),
    Archway     UMETA(DisplayName = "Stone Archway"),
    Pillar      UMETA(DisplayName = "Stone Pillar"),
    Wall        UMETA(DisplayName = "Stone Wall"),
    Platform    UMETA(DisplayName = "Stone Platform"),
    Ruin        UMETA(DisplayName = "Ancient Ruin")
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
    float WearLevel = 0.5f; // 0.0 = pristine, 1.0 = heavily weathered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMoss = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsClimbable = false;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        WearLevel = 0.5f;
        bHasMoss = true;
        bIsClimbable = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> StructureDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxStructuresPerBiome = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EBiomeType, int32> StructureCountPerBiome;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount = 20);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetStructuresInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatheringToStructure(AActor* StructureActor, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateArchitecturalLayout();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ClearAllStructures();

protected:
    UFUNCTION()
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);

    UFUNCTION()
    bool IsLocationSuitableForStructure(const FVector& Location, EArch_StructureType StructureType);

    UFUNCTION()
    UStaticMesh* GetMeshForStructureType(EArch_StructureType StructureType);

    UFUNCTION()
    void ConfigureStructureMaterial(UStaticMeshComponent* MeshComponent, const FArch_StructureData& StructureData);
};