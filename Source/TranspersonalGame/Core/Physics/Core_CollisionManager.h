#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "Core_CollisionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FName ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionResponse> ObjectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bGenerateOverlapEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bCanCharacterStepUpOn;

    FCore_CollisionProfile()
    {
        ProfileName = NAME_None;
        CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
        ObjectType = ECR_WorldStatic;
        bGenerateOverlapEvents = false;
        bCanCharacterStepUpOn = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    AActor* HitActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    UPrimitiveComponent* HitComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector ImpactNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Timestamp;

    FCore_CollisionData()
    {
        HitActor = nullptr;
        HitComponent = nullptr;
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactForce = 0.0f;
        Timestamp = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Collision Profile Management
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void SetCollisionProfile(UPrimitiveComponent* Component, const FCore_CollisionProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    FCore_CollisionProfile GetCollisionProfile(UPrimitiveComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void RegisterCollisionProfile(const FName& ProfileName, const FCore_CollisionProfile& Profile);

    // Collision Detection
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool LineTrace(const FVector& Start, const FVector& End, FHitResult& HitResult, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool SphereTrace(const FVector& Start, const FVector& End, float Radius, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool BoxTrace(const FVector& Start, const FVector& End, const FVector& HalfSize, const FRotator& Orientation, FHitResult& HitResult);

    // Overlap Detection
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool SphereOverlap(const FVector& Location, float Radius, TArray<AActor*>& OverlappingActors);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool BoxOverlap(const FVector& Location, const FVector& HalfSize, const FRotator& Orientation, TArray<AActor*>& OverlappingActors);

    // Collision Response
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void SetCollisionResponseToChannel(UPrimitiveComponent* Component, ECollisionChannel Channel, ECollisionResponse Response);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void SetCollisionResponseToAllChannels(UPrimitiveComponent* Component, ECollisionResponse Response);

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void SetPhysicsMaterial(UPrimitiveComponent* Component, class UPhysicalMaterial* PhysMaterial);

    // Collision Events
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
    void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Collision Data Management
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void RecordCollision(const FCore_CollisionData& CollisionData);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    TArray<FCore_CollisionData> GetRecentCollisions(float TimeWindow = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void ClearCollisionHistory();

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Core Collision", CallInEditor)
    void DebugDrawCollisionShapes();

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void SetDebugVisualization(bool bEnabled);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision")
    TMap<FName, FCore_CollisionProfile> RegisteredProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision")
    TArray<FCore_CollisionData> CollisionHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision")
    int32 MaxCollisionHistorySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision")
    bool bDebugVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Collision")
    float CollisionHistoryRetentionTime;

private:
    void InitializeDefaultProfiles();
    void CleanupOldCollisions();
    void DrawDebugCollisionShape(UPrimitiveComponent* Component);
};