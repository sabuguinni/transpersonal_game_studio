#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Arch_StructuralManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FString ElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float IntegrityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType AssociatedBiome;

    FArch_StructuralElement()
    {
        ElementName = TEXT("Unknown");
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        IntegrityLevel = 1.0f;
        AssociatedBiome = EBiomeType::Temperate;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorSpace
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString SpaceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector Dimensions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FString> ContainedObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float AmbientLightLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasRoof;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float StructuralSoundness;

    FArch_InteriorSpace()
    {
        SpaceName = TEXT("Unnamed Space");
        Dimensions = FVector(500.0f, 500.0f, 300.0f);
        AmbientLightLevel = 0.3f;
        bHasRoof = true;
        StructuralSoundness = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
    float BaseIntegrity;

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
    float GetStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FArch_StructuralElement> GetElementsByBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsStructureSafe() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetWeatheringEnabled(bool bEnabled);

private:
    void UpdateElementIntegrity(FArch_StructuralElement& Element, float DeltaTime);
    void CheckStructuralStability();
    float CalculateEnvironmentalDamage(const FArch_StructuralElement& Element) const;
};