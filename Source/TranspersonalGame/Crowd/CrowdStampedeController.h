#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing       UMETA(DisplayName = "Grazing"),
    Wandering     UMETA(DisplayName = "Wandering"),
    Alerted       UMETA(DisplayName = "Alerted"),
    Stampeding    UMETA(DisplayName = "Stampeding"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Resting       UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ECrowd_HerbivoreSpecies : uint8
{
    Triceratops   UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Stegosaurus   UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus  UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float Health;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerbivoreSpecies Species;

    FCrowd_HerdMember()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , Health(100.f)
        , bIsAlpha(false)
        , Species(ECrowd_HerbivoreSpecies::Triceratops)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector ThreatLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector FleeDirection;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Duration;

    FCrowd_StampedeData()
        : ThreatLocation(FVector::ZeroVector)
        , FleeDirection(FVector::ZeroVector)
        , StampedeSpeed(800.f)
        , PanicRadius(2000.f)
        , Duration(15.f)
    {}
};

UCLASS(ClassGroup = (Crowd), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdHerdComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdHerdComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState HerdState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerbivoreSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<FCrowd_HerdMember> HerdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FCrowd_StampedeData ActiveStampede;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsStampeding;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void InitializeHerd(ECrowd_HerbivoreSpecies InSpecies, int32 InSize, FVector CenterLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void CalmHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void UpdateFlockingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector CalculateCohesionForce(int32 MemberIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector CalculateSeparationForce(int32 MemberIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector CalculateAlignmentForce(int32 MemberIndex) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float StampedeTimer;
    float GrazingTimer;
};

UCLASS(ClassGroup = (Crowd), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdStampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdStampedeController();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* ThreatDetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Components",
        meta = (AllowPrivateAccess = "true"))
    UCrowdHerdComponent* HerdComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float GlobalThreatRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxSimultaneousHerds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    TArray<ACrowdStampedeController*> NearbyHerds;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void BroadcastThreatToNearbyHerds(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void OnPredatorEnterRange(AActor* PredatorActor);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void SpawnHerdAtLocation(FVector Location, ECrowd_HerbivoreSpecies Species, int32 Count);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnThreatSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
