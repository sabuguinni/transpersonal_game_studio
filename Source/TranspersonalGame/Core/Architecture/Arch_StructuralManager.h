#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Arch_StructuralManager.generated.h"

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
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsWeathered;

    FArch_StructuralElement()
    {
        ElementName = TEXT("Stone_Structure");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savana;
        StructuralIntegrity = 100.0f;
        bIsWeathered = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorSpace
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString SpaceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float SpaceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FString> InteriorElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasShelter;

    FArch_InteriorSpace()
    {
        SpaceName = TEXT("Cave_Dwelling");
        CenterLocation = FVector::ZeroVector;
        SpaceRadius = 500.0f;
        bHasFirePit = false;
        bHasShelter = false;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructuralElement> StructuralElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_InteriorSpace> InteriorSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatheringRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bEnableWeathering;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructuralElement(const FArch_StructuralElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void CreateInteriorSpace(const FArch_InteriorSpace& Space);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructuralElement> GetStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void GenerateArchitecturalElements();

private:
    void InitializeDefaultStructures();
    FVector GetBiomeBaseLocation(EBiomeType BiomeType);
};