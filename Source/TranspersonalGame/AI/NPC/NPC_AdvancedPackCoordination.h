#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_AdvancedPackCoordination.generated.h"

UENUM(BlueprintType)
enum class ENPC_PackFormation : uint8
{
    None = 0,
    Line,
    Wedge,
    Circle,
    Ambush,
    Hunting,
    Defensive
};

UENUM(BlueprintType)
enum class ENPC_PackRole : uint8
{
    None = 0,
    Alpha,
    Beta,
    Scout,
    Hunter,
    Defender,
    Juvenile
};

USTRUCT(BlueprintType)
struct FNPC_PackMember
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    class APawn* MemberPawn;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ENPC_PackRole Role;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector RelativePosition;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float DistanceFromAlpha;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Stamina;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float Aggression;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bIsInjured;

    FNPC_PackMember()
    {
        MemberPawn = nullptr;
        Role = ENPC_PackRole::None;
        RelativePosition = FVector::ZeroVector;
        DistanceFromAlpha = 0.0f;
        Stamina = 100.0f;
        Aggression = 50.0f;
        bIsInjured = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_PackCoordinationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    ENPC_PackFormation CurrentFormation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FVector TargetLocation;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    class AActor* TargetActor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float FormationRadius;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float MovementSpeed;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bIsHunting;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bIsDefending;

    FNPC_PackCoordinationData()
    {
        CurrentFormation = ENPC_PackFormation::None;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        FormationRadius = 500.0f;
        MovementSpeed = 300.0f;
        bIsHunting = false;
        bIsDefending = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_AdvancedPackCoordination : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_AdvancedPackCoordination();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void InitializePack(APawn* AlphaPawn);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AddPackMember(APawn* NewMember, ENPC_PackRole Role);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void RemovePackMember(APawn* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void SetPackFormation(ENPC_PackFormation NewFormation);

    // Coordination Commands
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void MoveToLocation(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void DefendPosition(FVector DefensePosition);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void ScatterFormation();

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void ReformPack();

    // Pack Status
    UFUNCTION(BlueprintPure, Category = "Pack Coordination")
    int32 GetPackSize() const { return PackMembers.Num(); }

    UFUNCTION(BlueprintPure, Category = "Pack Coordination")
    APawn* GetAlphaPawn() const { return AlphaPawn; }

    UFUNCTION(BlueprintPure, Category = "Pack Coordination")
    ENPC_PackFormation GetCurrentFormation() const { return CoordinationData.CurrentFormation; }

    UFUNCTION(BlueprintPure, Category = "Pack Coordination")
    bool IsPackActive() const { return PackMembers.Num() > 0 && AlphaPawn != nullptr; }

protected:
    // Pack Data
    UPROPERTY(BlueprintReadOnly, Category = "Pack Coordination", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_PackMember> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Coordination", meta = (AllowPrivateAccess = "true"))
    APawn* AlphaPawn;

    UPROPERTY(BlueprintReadWrite, Category = "Pack Coordination", meta = (AllowPrivateAccess = "true"))
    FNPC_PackCoordinationData CoordinationData;

    // Formation Management
    void UpdateFormationPositions();
    void CalculateLineFormation();
    void CalculateWedgeFormation();
    void CalculateCircleFormation();
    void CalculateAmbushFormation();
    void CalculateHuntingFormation();
    void CalculateDefensiveFormation();

    // Pack Behavior
    void UpdatePackBehavior(float DeltaTime);
    void HandlePackCommunication();
    void AssignPackRoles();
    void MonitorPackHealth();

    // Formation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings", meta = (AllowPrivateAccess = "true"))
    float MaxFormationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings", meta = (AllowPrivateAccess = "true"))
    float FormationUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings", meta = (AllowPrivateAccess = "true"))
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoReform;

private:
    float LastFormationUpdate;
    float LastCommunicationUpdate;
};