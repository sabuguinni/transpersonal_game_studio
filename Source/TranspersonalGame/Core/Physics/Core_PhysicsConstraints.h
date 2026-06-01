#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Core_PhysicsConstraints.generated.h"

UENUM(BlueprintType)
enum class ECore_ConstraintType : uint8
{
    Fixed,
    Hinge,
    Prismatic,
    Ball,
    Cone,
    Custom
};

USTRUCT(BlueprintType)
struct FCore_ConstraintSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
    ECore_ConstraintType ConstraintType = ECore_ConstraintType::Fixed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
    bool bEnableLinearLimits = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
    FVector LinearLimitSize = FVector(100.0f, 100.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
    bool bEnableAngularLimits = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
    float AngularLimitAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
    bool bEnableCollision = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
    float BreakForce = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
    float BreakTorque = 10000.0f;

    FCore_ConstraintSettings()
    {
        ConstraintType = ECore_ConstraintType::Fixed;
        bEnableLinearLimits = false;
        LinearLimitSize = FVector(100.0f, 100.0f, 100.0f);
        bEnableAngularLimits = false;
        AngularLimitAngle = 45.0f;
        bEnableCollision = false;
        BreakForce = 10000.0f;
        BreakTorque = 10000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsConstraints : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsConstraints();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    TArray<FCore_ConstraintSettings> ConstraintSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Constraints")
    TArray<UPhysicsConstraintComponent*> ActiveConstraints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    bool bAutoCreateConstraints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraints")
    float DefaultBreakForce;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    UPhysicsConstraintComponent* CreateConstraint(const FCore_ConstraintSettings& Settings, 
                                                  AActor* Actor1, AActor* Actor2);

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    void RemoveConstraint(UPhysicsConstraintComponent* Constraint);

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    void RemoveAllConstraints();

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    void SetConstraintBreakForce(UPhysicsConstraintComponent* Constraint, float NewBreakForce);

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    void SetConstraintBreakTorque(UPhysicsConstraintComponent* Constraint, float NewBreakTorque);

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    TArray<UPhysicsConstraintComponent*> GetActiveConstraints() const;

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    int32 GetConstraintCount() const;

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    bool IsConstraintBroken(UPhysicsConstraintComponent* Constraint) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Constraints")
    void OnConstraintBroken(UPhysicsConstraintComponent* BrokenConstraint);

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    void CreateRagdollConstraints(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Constraints")
    void CreateChainConstraints(const TArray<AActor*>& ChainActors);

private:
    void SetupConstraintFromSettings(UPhysicsConstraintComponent* Constraint, const FCore_ConstraintSettings& Settings);
    void CheckForBrokenConstraints();
    UPhysicsConstraintComponent* CreateConstraintComponent();
};