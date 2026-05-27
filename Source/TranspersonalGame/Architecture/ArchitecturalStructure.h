#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructure.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Shelter         UMETA(DisplayName = "Shelter"),
    Monument        UMETA(DisplayName = "Monument"),
    Wall            UMETA(DisplayName = "Wall"),
    Pillar          UMETA(DisplayName = "Pillar"),
    Platform        UMETA(DisplayName = "Platform"),
    Bridge          UMETA(DisplayName = "Bridge")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Shelter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DurabilityMax = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DurabilityCurrent = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bCanProvideShade = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bCanProvideShelter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float ShelterRadius = 500.0f;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::Shelter;
        DurabilityMax = 1000.0f;
        DurabilityCurrent = 1000.0f;
        bCanProvideShade = true;
        bCanProvideShelter = true;
        ShelterRadius = 500.0f;
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
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Data")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintPure, Category = "Structure")
    float GetDurabilityPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Structure")
    bool IsInShelterRange(FVector TestLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnStructureDestroyed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnPlayerEnterShelter();

    UFUNCTION(BlueprintImplementableEvent, Category = "Structure")
    void OnPlayerExitShelter();

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bPlayerInShelter = false;
    float WeatherDecayRate = 1.0f;
};