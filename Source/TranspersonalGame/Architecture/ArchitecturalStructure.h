#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "ArchitecturalStructure.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Pillar      UMETA(DisplayName = "Stone Pillar"),
    Archway     UMETA(DisplayName = "Stone Archway"),
    Wall        UMETA(DisplayName = "Stone Wall"),
    Ruins       UMETA(DisplayName = "Ancient Ruins"),
    Shelter     UMETA(DisplayName = "Primitive Shelter"),
    Platform    UMETA(DisplayName = "Stone Platform")
};

UENUM(BlueprintType)
enum class EArch_StructureCondition : uint8
{
    Pristine    UMETA(DisplayName = "Pristine"),
    Weathered   UMETA(DisplayName = "Weathered"),
    Damaged     UMETA(DisplayName = "Damaged"),
    Ruined      UMETA(DisplayName = "Ruined"),
    Overgrown   UMETA(DisplayName = "Overgrown")
};

USTRUCT(BlueprintType)
struct FArch_StructureProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Pillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureCondition Condition = EArch_StructureCondition::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bCanClimb = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvidesShade = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bCanHideInside = false;

    FArch_StructureProperties()
    {
        StructureType = EArch_StructureType::Pillar;
        Condition = EArch_StructureCondition::Weathered;
        StructuralIntegrity = 75.0f;
        bCanClimb = false;
        bProvidesShade = true;
        bCanHideInside = false;
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
    class UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureProperties Properties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FString StructureName = TEXT("Ancient Structure");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FText StructureDescription = FText::FromString(TEXT("A weathered stone structure from a bygone era."));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float InteractionRange = 300.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArch_StructureType Type, EArch_StructureCondition InitialCondition);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanPlayerInteract() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void OnPlayerEnterRange();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void OnPlayerExitRange();

    UFUNCTION(BlueprintPure, Category = "Architecture")
    FArch_StructureProperties GetStructureProperties() const { return Properties; }

    UFUNCTION(BlueprintPure, Category = "Architecture")
    EArch_StructureType GetStructureType() const { return Properties.StructureType; }

    UFUNCTION(BlueprintPure, Category = "Architecture")
    float GetStructuralIntegrity() const { return Properties.StructuralIntegrity; }

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void SetupStructureMesh();
    void ApplyWeatheringEffects();
    void UpdateStructureCondition();

    bool bPlayerInRange = false;
    float TimeInCurrentCondition = 0.0f;
};

#include "ArchitecturalStructure.generated.h"