#include "SurvivalFootIKSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

USurvivalFootIKSystem::USurvivalFootIKSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Initialize settings
    IKSettings = FAnim_FootIKSettings();
    
    // Initialize socket names
    LeftFootSocketName = FName("foot_l");
    RightFootSocketName = FName("foot_r");
    
    // Initialize IK data
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
    PelvisOffset = 0.0f;
    TargetPelvisOffset = 0.0f;
}

void USurvivalFootIKSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void USurvivalFootIKSystem::InitializeComponent()
{
    // Get the skeletal mesh component from the owner
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (!OwnerMesh)
        {
            // Try to get it from character
            ACharacter* Character = Cast<ACharacter>(Owner);
            if (Character)
            {
                OwnerMesh = Character->GetMesh();
            }
        }
    }

    if (!OwnerMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalFootIKSystem: Could not find SkeletalMeshComponent on owner"));
    }
}

void USurvivalFootIKSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (OwnerMesh && OwnerMesh->GetWorld())
    {
        UpdateFootIK(DeltaTime);
    }
}

void USurvivalFootIKSystem::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMesh)
    {
        return;
    }

    // Get root location
    FVector RootLocation = OwnerMesh->GetComponentLocation();
    
    // Calculate IK for both feet
    LeftFootIK = CalculateFootIK(LeftFootSocketName, RootLocation);
    RightFootIK = CalculateFootIK(RightFootSocketName, RootLocation);
    
    // Update pelvis offset
    UpdatePelvisOffset(DeltaTime);
    
    // Draw debug info if enabled
    if (IKSettings.bEnableDebugDraw)
    {
        DrawDebugInfo();
    }
}

FAnim_FootIKData USurvivalFootIKSystem::CalculateFootIK(const FName& SocketName, const FVector& RootLocation)
{
    FAnim_FootIKData FootData;
    
    if (!OwnerMesh)
    {
        return FootData;
    }

    // Get foot socket location
    FVector SocketLocation = OwnerMesh->GetSocketLocation(SocketName);
    if (SocketLocation.IsZero())
    {
        // Fallback to bone location if socket doesn't exist
        SocketLocation = OwnerMesh->GetBoneLocation(SocketName);
    }

    // Trace for ground
    FVector HitLocation;
    FVector HitNormal;
    bool bHitGround = TraceForGround(SocketLocation, HitLocation, HitNormal);

    if (bHitGround)
    {
        // Calculate distance from current foot to ground
        float DistanceToGround = SocketLocation.Z - HitLocation.Z;
        FootData.DistanceFromGround = DistanceToGround;

        // Only apply IK if the distance exceeds threshold
        if (FMath::Abs(DistanceToGround) > IKSettings.IKThreshold)
        {
            // Calculate foot position adjustment
            FootData.FootLocation = FVector(0, 0, -DistanceToGround);
            
            // Calculate foot rotation to match ground normal
            FVector ForwardVector = OwnerMesh->GetForwardVector();
            FVector RightVector = FVector::CrossProduct(HitNormal, ForwardVector).GetSafeNormal();
            ForwardVector = FVector::CrossProduct(RightVector, HitNormal).GetSafeNormal();
            
            FootData.FootRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitNormal);
            FootData.IKAlpha = 1.0f;
        }
        else
        {
            // Ground is close enough, no IK needed
            FootData.IKAlpha = 0.0f;
        }
    }
    else
    {
        // No ground hit, disable IK
        FootData.IKAlpha = 0.0f;
    }

    return FootData;
}

bool USurvivalFootIKSystem::TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!OwnerMesh || !OwnerMesh->GetWorld())
    {
        return false;
    }

    UWorld* World = OwnerMesh->GetWorld();
    
    // Setup trace parameters
    FVector TraceStart = StartLocation + FVector(0, 0, IKSettings.TraceDistance * 0.5f);
    FVector TraceEnd = StartLocation - FVector(0, 0, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = false;
    QueryParams.bReturnPhysicalMaterial = false;

    // Perform line trace
    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit)
    {
        OutHitLocation = HitResult.Location;
        OutHitNormal = HitResult.Normal;
        return true;
    }

    return false;
}

void USurvivalFootIKSystem::UpdatePelvisOffset(float DeltaTime)
{
    // Calculate target pelvis offset based on foot IK
    float LeftOffset = (LeftFootIK.IKAlpha > 0.0f) ? LeftFootIK.FootLocation.Z : 0.0f;
    float RightOffset = (RightFootIK.IKAlpha > 0.0f) ? RightFootIK.FootLocation.Z : 0.0f;
    
    // Use the higher foot as reference (less negative value)
    TargetPelvisOffset = FMath::Max(LeftOffset, RightOffset);
    
    // Smooth interpolation to target
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, DeltaTime, IKSettings.InterpSpeed);
}

void USurvivalFootIKSystem::DrawDebugInfo()
{
    if (!OwnerMesh || !OwnerMesh->GetWorld())
    {
        return;
    }

    UWorld* World = OwnerMesh->GetWorld();
    
    // Draw foot IK debug info
    if (LeftFootIK.IKAlpha > 0.0f)
    {
        FVector LeftFootPos = OwnerMesh->GetSocketLocation(LeftFootSocketName);
        DrawDebugSphere(World, LeftFootPos + LeftFootIK.FootLocation, 5.0f, 8, FColor::Green, false, 0.1f);
        DrawDebugLine(World, LeftFootPos, LeftFootPos + LeftFootIK.FootLocation, FColor::Green, false, 0.1f, 0, 2.0f);
    }
    
    if (RightFootIK.IKAlpha > 0.0f)
    {
        FVector RightFootPos = OwnerMesh->GetSocketLocation(RightFootSocketName);
        DrawDebugSphere(World, RightFootPos + RightFootIK.FootLocation, 5.0f, 8, FColor::Blue, false, 0.1f);
        DrawDebugLine(World, RightFootPos, RightFootPos + RightFootIK.FootLocation, FColor::Blue, false, 0.1f, 0, 2.0f);
    }
    
    // Draw pelvis offset
    if (FMath::Abs(PelvisOffset) > 0.1f)
    {
        FVector PelvisPos = OwnerMesh->GetComponentLocation();
        DrawDebugSphere(World, PelvisPos + FVector(0, 0, PelvisOffset), 8.0f, 8, FColor::Red, false, 0.1f);
    }
}

void USurvivalFootIKSystem::SetIKSettings(const FAnim_FootIKSettings& NewSettings)
{
    IKSettings = NewSettings;
}