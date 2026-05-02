#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerSphere.h"
#include "../SharedTypes.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave         UMETA(DisplayName = "Cave Shelter"),
    TreePlatform UMETA(DisplayName = "Tree Platform"),
    CliffDwelling UMETA(DisplayName = "Cliff Dwelling"),
    Underground  UMETA(DisplayName = "Underground Bunker")
};

UENUM(BlueprintType)
enum class EArch_ShelterComponent : uint8
{
    Wall         UMETA(DisplayName = "Wall"),
    Roof         UMETA(DisplayName = "Roof"),
    Floor        UMETA(DisplayName = "Floor"),
    Door         UMETA(DisplayName = "Door"),
    Window       UMETA(DisplayName = "Window"),
    FirePit      UMETA(DisplayName = "Fire Pit"),
    Storage      UMETA(DisplayName = "Storage"),
    Ladder       UMETA(DisplayName = "Ladder"),
    Bridge       UMETA(DisplayName = "Bridge"),
    Platform     UMETA(DisplayName = "Platform")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float StorageCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    TArray<EArch_ShelterComponent> BuiltComponents;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::Cave;
        Location = FVector::ZeroVector;
        ProtectionLevel = 0.0f;
        StorageCapacity = 0.0f;
        bHasFirePit = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    EArch_ShelterType ShelterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    float MaxProtectionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    float MaxStorageCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<EArch_ShelterComponent> RequiredComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter State")
    FArch_ShelterData CurrentShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Components")
    TMap<EArch_ShelterComponent, UStaticMeshComponent*> ComponentMeshes;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanBuildComponent(EArch_ShelterComponent ComponentType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool BuildComponent(EArch_ShelterComponent ComponentType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RemoveComponent(EArch_ShelterComponent ComponentType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetShelterProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetStorageCapacity() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool HasFirePit() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsComponentBuilt(EArch_ShelterComponent ComponentType) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<EArch_ShelterComponent> GetMissingComponents() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnComponentBuilt(EArch_ShelterComponent ComponentType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnShelterCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnPlayerEnterShelter();

    UFUNCTION(BlueprintImplementableEvent, Category = "Shelter")
    void OnPlayerExitShelter();

private:
    void InitializeComponentMeshes();
    void UpdateShelterStats();
    bool ValidateComponentPlacement(EArch_ShelterComponent ComponentType);

    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};