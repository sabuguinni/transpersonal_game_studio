#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Quest_CraftingSystem.h"
#include "Quest_ResourcePickupActor.generated.h"

UCLASS()
class TRANSPERSONALGAME_API AQuest_ResourcePickupActor : public AActor
{
    GENERATED_BODY()
    
public:    
    AQuest_ResourcePickupActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float PickupRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bAutoPickup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FVector MeshScale;

public:
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void SetResourceType(EQuest_ResourceType NewResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource")
    EQuest_ResourceType GetResourceType() const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 GetResourceAmount() const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool CanBePickedUp() const;

    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PickupResource(AActor* PickupActor);

protected:
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void SetupMeshForResourceType();
    void ConfigureCollision();
};