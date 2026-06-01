#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Quest_ResourcePickup.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourcePickup : public AStaticMeshActor
{
    GENERATED_BODY()

public:
    AQuest_ResourcePickup();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
    class USphereComponent* PickupCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 ResourceAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTime;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    bool bIsPickedUp;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    float RespawnTimer;

public:
    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void PickupResource(class ATranspersonalCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    void RespawnResource();

    UFUNCTION()
    void OnPickupOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    virtual void Tick(float DeltaTime) override;

    FORCEINLINE EResourceType GetResourceType() const { return ResourceType; }
    FORCEINLINE int32 GetResourceAmount() const { return ResourceAmount; }
    FORCEINLINE bool IsPickedUp() const { return bIsPickedUp; }
};