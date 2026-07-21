#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/ActorComponent.h"
#include "Core_MaterialPhysics.generated.h"

/**
 * Physical material types for different terrain and object surfaces
 * Used to define physics behavior for different materials in the prehistoric world
 */
UENUM(BlueprintType)
enum class ECore_MaterialType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Dirt        UMETA(DisplayName = "Dirt"),
    Sand        UMETA(DisplayName = "Sand"),
    Grass       UMETA(DisplayName = "Grass"),
    Wood        UMETA(DisplayName = "Wood"),
    Water       UMETA(DisplayName = "Water"),
    Mud         UMETA(DisplayName = "Mud"),
    Stone       UMETA(DisplayName = "Stone"),
    Bone        UMETA(DisplayName = "Bone"),
    Flesh       UMETA(DisplayName = "Flesh")
};

/**
 * Material physics properties for realistic surface interactions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_MaterialProperties
{
    GENERATED_BODY()

    /** Friction coefficient (0.0 = no friction, 1.0 = high friction) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Friction = 0.7f;

    /** Restitution/bounciness (0.0 = no bounce, 1.0 = perfect bounce) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Restitution = 0.3f;

    /** Density in kg/m³ */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.1"))
    float Density = 1000.0f;

    /** Sound dampening factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SoundDampening = 0.5f;

    /** Footstep volume modifier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FootstepVolume = 1.0f;

    /** Can this material be damaged/destroyed? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCanBeDestroyed = true;

    /** Hardness factor for destruction calculations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.1"))
    float Hardness = 1.0f;

    FCore_MaterialProperties()
    {
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1000.0f;
        SoundDampening = 0.5f;
        FootstepVolume = 1.0f;
        bCanBeDestroyed = true;
        Hardness = 1.0f;
    }
};

/**
 * Material Physics Component
 * Manages physical material properties and interactions for objects in the prehistoric world
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_MaterialPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_MaterialPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Current material type */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Physics")
    ECore_MaterialType MaterialType = ECore_MaterialType::Rock;

    /** Material properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Physics")
    FCore_MaterialProperties MaterialProperties;

    /** Physical material asset reference */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Physics")
    TObjectPtr<UPhysicalMaterial> PhysicalMaterial;

    /** Apply material properties to the owner actor */
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void ApplyMaterialProperties();

    /** Get material properties by type */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Material Physics")
    static FCore_MaterialProperties GetMaterialPropertiesByType(ECore_MaterialType InMaterialType);

    /** Set material type and update properties */
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void SetMaterialType(ECore_MaterialType NewMaterialType);

    /** Get friction coefficient for surface interactions */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Material Physics")
    float GetFriction() const { return MaterialProperties.Friction; }

    /** Get restitution for bounce calculations */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Material Physics")
    float GetRestitution() const { return MaterialProperties.Restitution; }

    /** Get material density */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Material Physics")
    float GetDensity() const { return MaterialProperties.Density; }

    /** Check if material can be destroyed */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Material Physics")
    bool CanBeDestroyed() const { return MaterialProperties.bCanBeDestroyed; }

    /** Get hardness for destruction calculations */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Material Physics")
    float GetHardness() const { return MaterialProperties.Hardness; }

    /** Calculate impact force based on velocity and mass */
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    float CalculateImpactForce(const FVector& Velocity, float Mass) const;

    /** Handle collision with another material */
    UFUNCTION(BlueprintCallable, Category = "Material Physics")
    void HandleMaterialCollision(UCore_MaterialPhysics* OtherMaterial, const FVector& ImpactPoint, float ImpactForce);

private:
    /** Update physical material properties */
    void UpdatePhysicalMaterial();

    /** Create physical material if needed */
    void CreatePhysicalMaterial();
};

#include "Core_MaterialPhysics.generated.h"