#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "../../SharedTypes.h"
#include "PrehistoricShelterSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    None            UMETA(DisplayName = "None"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang"),
    StoneArchway    UMETA(DisplayName = "Stone Archway"),
    BoulderCluster  UMETA(DisplayName = "Boulder Cluster"),
    NaturalBridge   UMETA(DisplayName = "Natural Bridge")
};

USTRUCT(BlueprintType)
struct FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureBonus = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bCanStartFire = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    FArch_ShelterProperties()
    {
        ShelterType = EArch_ShelterType::None;
        ProtectionRadius = 500.0f;
        WeatherProtection = 0.8f;
        TemperatureBonus = 5.0f;
        bCanStartFire = true;
        MaxOccupants = 4;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_ShelterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_ShelterComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter", meta = (AllowPrivateAccess = "true"))
    FArch_ShelterProperties ShelterProperties;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shelter", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> CurrentOccupants;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shelter", meta = (AllowPrivateAccess = "true"))
    bool bIsOccupied = false;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanEnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool EnterShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool ExitShelter(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetWeatherProtection() const { return ShelterProperties.WeatherProtection; }

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureBonus() const { return ShelterProperties.TemperatureBonus; }

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanStartFire() const { return ShelterProperties.bCanStartFire; }

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    int32 GetAvailableSpace() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<AActor*> GetCurrentOccupants() const { return CurrentOccupants; }

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterType(EArch_ShelterType NewType);

private:
    void UpdateShelterEffects();
    bool IsActorInRange(AActor* Actor) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_PrehistoricShelter : public AActor
{
    GENERATED_BODY()

public:
    AArch_PrehistoricShelter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UArch_ShelterComponent* ShelterComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EBiomeType AssociatedBiome = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bIsNaturalFormation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructuralIntegrity = 1.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeShelter(EArch_ShelterType ShelterType, EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsStructurallySafe() const { return StructuralIntegrity > 0.5f; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnShelterEntered(AActor* Actor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnShelterExited(AActor* Actor);

private:
    void SetupMeshForShelterType(EArch_ShelterType ShelterType);
    void ApplyBiomeSpecificProperties(EBiomeType Biome);
};

UCLASS()
class TRANSPERSONALGAME_API UArch_ShelterManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterShelter(AArch_PrehistoricShelter* Shelter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UnregisterShelter(AArch_PrehistoricShelter* Shelter);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AArch_PrehistoricShelter*> FindNearestShelters(FVector Location, float SearchRadius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AArch_PrehistoricShelter* FindBestShelterForActor(AActor* Actor, float SearchRadius = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    int32 GetTotalShelterCount() const { return RegisteredShelters.Num(); }

protected:
    UPROPERTY()
    TArray<AArch_PrehistoricShelter*> RegisteredShelters;

private:
    void CleanupInvalidShelters();
};