#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/CollisionProfile.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Core_CollisionLayerManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FName LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECollisionChannel CollisionChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECollisionResponse DefaultResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<FName> BlockingLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<FName> IgnoringLayers;

    FCore_CollisionLayer()
    {
        LayerName = NAME_None;
        CollisionChannel = ECC_WorldStatic;
        DefaultResponse = ECR_Block;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsMaterial
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    FName MaterialName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Friction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Restitution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Density;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    bool bOverrideFrictionCombineMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    TEnumAsByte<EFrictionCombineMode::Type> FrictionCombineMode;

    FCore_PhysicsMaterial()
    {
        MaterialName = NAME_None;
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        bOverrideFrictionCombineMode = false;
        FrictionCombineMode = EFrictionCombineMode::Average;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_CollisionLayerManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_CollisionLayerManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Collision Layer Management
    UFUNCTION(BlueprintCallable, Category = "Collision Layers")
    void InitializeCollisionLayers();

    UFUNCTION(BlueprintCallable, Category = "Collision Layers")
    bool RegisterCollisionLayer(const FCore_CollisionLayer& Layer);

    UFUNCTION(BlueprintCallable, Category = "Collision Layers")
    FCore_CollisionLayer GetCollisionLayer(FName LayerName) const;

    UFUNCTION(BlueprintCallable, Category = "Collision Layers")
    ECollisionChannel GetCollisionChannel(FName LayerName) const;

    UFUNCTION(BlueprintCallable, Category = "Collision Layers")
    void SetCollisionResponseBetweenLayers(FName Layer1, FName Layer2, ECollisionResponse Response);

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    void InitializePhysicsMaterials();

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    bool RegisterPhysicsMaterial(const FCore_PhysicsMaterial& Material);

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    FCore_PhysicsMaterial GetPhysicsMaterial(FName MaterialName) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    class UPhysicalMaterial* CreateUE5PhysicsMaterial(FName MaterialName);

    // Actor Collision Setup
    UFUNCTION(BlueprintCallable, Category = "Collision Setup")
    void SetActorCollisionLayer(AActor* Actor, FName LayerName);

    UFUNCTION(BlueprintCallable, Category = "Collision Setup")
    void SetActorPhysicsMaterial(AActor* Actor, FName MaterialName);

    UFUNCTION(BlueprintCallable, Category = "Collision Setup")
    void ApplyCollisionProfileToActor(AActor* Actor, FName ProfileName);

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void ValidateCollisionSetup();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void LogCollisionLayers();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void LogPhysicsMaterials();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Layers")
    TArray<FCore_CollisionLayer> CollisionLayers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Materials")
    TArray<FCore_PhysicsMaterial> PhysicsMaterials;

    UPROPERTY()
    TMap<FName, class UPhysicalMaterial*> CreatedPhysicsMaterials;

private:
    void CreateDefaultCollisionLayers();
    void CreateDefaultPhysicsMaterials();
    void SetupCollisionResponses();
};