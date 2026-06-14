#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StoneArchway     UMETA(DisplayName = "Stone Archway"),
    CaveEntrance     UMETA(DisplayName = "Cave Entrance"),
    RockShelter      UMETA(DisplayName = "Rock Shelter"),
    StonePlatform    UMETA(DisplayName = "Stone Platform"),
    AncientRuins     UMETA(DisplayName = "Ancient Ruins")
};

USTRUCT(BlueprintType)
struct FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StoneArchway;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float ProtectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float TemperatureModifier = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bProvidesWeatherProtection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 4;

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::StoneArchway;
        ProtectionRadius = 500.0f;
        TemperatureModifier = 5.0f;
        bProvidesWeatherProtection = true;
        MaxOccupants = 4;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ProtectionZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    TArray<AActor*> CurrentOccupants;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool CanEnterStructure(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool EnterStructure(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool ExitStructure(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    float GetProtectionLevel(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    bool IsLocationProtected(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    TArray<AActor*> GetCurrentOccupants() const { return CurrentOccupants; }

protected:
    UFUNCTION()
    void OnProtectionZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnProtectionZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateStructureMesh();
    void UpdateProtectionRadius();
};