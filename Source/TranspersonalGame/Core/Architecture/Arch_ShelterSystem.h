#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Arch_ShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneHut        UMETA(DisplayName = "Stone Hut"),
    WoodLean        UMETA(DisplayName = "Wood Lean-to"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    BoneFrame       UMETA(DisplayName = "Bone Frame"),
    HideWall        UMETA(DisplayName = "Hide Wall")
};

UENUM(BlueprintType)
enum class EArch_ShelterState : uint8
{
    Intact          UMETA(DisplayName = "Intact"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruined          UMETA(DisplayName = "Ruined"),
    Collapsed       UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::StoneHut;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterState CurrentState = EArch_ShelterState::Intact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherResistance = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float InsulationValue = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorageArea = false;

    FArch_ShelterConfig()
    {
        ShelterType = EArch_ShelterType::StoneHut;
        CurrentState = EArch_ShelterState::Intact;
        StructuralIntegrity = 100.0f;
        WeatherResistance = 80.0f;
        InsulationValue = 60.0f;
        MaxOccupants = 2;
        bHasFirePit = false;
        bHasStorageArea = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WallMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RoofMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterConfig ShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WeatherDamageRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RepairEfficiency = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occupancy")
    TArray<AActor*> CurrentOccupants;

    // Core shelter functionality
    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void InitializeShelter(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RepairShelter(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanEnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetShelterEffectiveness() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void UpdateShelterState();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetFirePit(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetStorageArea(bool bEnabled);

private:
    void UpdateMeshVisibility();
    void ApplyStateEffects();
    float CalculateWeatherProtection() const;
    void ProcessDegradation(float DeltaTime);
};