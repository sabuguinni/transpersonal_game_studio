#include "Anim_IKFootPlacement.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

UAnim_IKFootPlacement::UAnim_IKFootPlacement()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default settings
    IKSettings = FAnim_IKSettings();
    bIKEnabled = true;
    
    // Default bone names (can be overridden in Blueprint)
    LeftFootBoneName = FName("foot_l");
    RightFootBoneName = FName("foot_r");
    PelvisBoneName = FName("pelvis");
    
    // Initialize IK data
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
    PelvisOffset = 0.0f;
    
    // Internal state
    LastUpdateTime = 0.0f;
    LastOwnerLocation = FVector::ZeroVector;
    bNeedsUpdate = true;
    
    OwnerMesh = nullptr;
    AnimInstance = nullptr;
}

void UAnim_IKFootPlacement::BeginPlay()
{
    Super::BeginPlay();
    InitializeComponent();
}

void UAnim_IKFootPlacement::InitializeComponent()
{
    CacheComponentReferences();
    
    if (OwnerMesh && AnimInstance)
    {
        UE_LOG(LogTemp, Log, TEXT("IK Foot Placement initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("IK Foot Placement failed to initialize - missing mesh or anim instance"));
    }
}

void UAnim_IKFootPlacement::CacheComponentReferences()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Try to get skeletal mesh component
    OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!OwnerMesh)
    {
        // If owner is a character, get the mesh component
        if (ACharacter* Character = Cast<ACharacter>(Owner))
        {
            OwnerMesh = Character->GetMesh();
        }
    }
    
    if (OwnerMesh)
    {
        AnimInstance = OwnerMesh->GetAnimInstance();
    }
}

void UAnim_IKFootPlacement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIKEnabled && OwnerMesh && AnimInstance)
    {
        UpdateFootIK(DeltaTime);
    }
}

void UAnim_IKFootPlacement::UpdateFootIK(float DeltaTime)
{
    if (!OwnerMesh || !AnimInstance)
    {
        return;
    }
    
    // Check if we need to update (performance optimization)
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    float DistanceMoved = FVector::Dist(CurrentLocation, LastOwnerLocation);
    
    if (DistanceMoved < 1.0f && (GetWorld()->GetTimeSeconds() - LastUpdateTime) < 0.1f)
    {
        return; // Skip update if character hasn't moved much
    }
    
    LastOwnerLocation = CurrentLocation;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Get foot socket locations
    FVector LeftFootLocation = GetSocketLocation(LeftFootBoneName);
    FVector RightFootLocation = GetSocketLocation(RightFootBoneName);
    
    // Calculate IK data for both feet
    FAnim_FootIKData TargetLeftIK = GetFootIKData(LeftFootBoneName, LeftFootLocation);
    FAnim_FootIKData TargetRightIK = GetFootIKData(RightFootBoneName, RightFootLocation);
    
    // Interpolate to target values
    InterpolateIKData(LeftFootIK, TargetLeftIK, DeltaTime);
    InterpolateIKData(RightFootIK, TargetRightIK, DeltaTime);
    
    // Update pelvis offset
    UpdatePelvisOffset();
    
    // Debug drawing
    if (IKSettings.bEnableDebugDraw)
    {
        DrawDebugInfo();
    }
}

FAnim_FootIKData UAnim_IKFootPlacement::GetFootIKData(const FName& FootBoneName, const FVector& FootSocketLocation)
{
    FAnim_FootIKData IKData;
    
    if (!OwnerMesh)
    {
        return IKData;
    }
    
    // Perform ground trace
    FVector HitLocation;
    FVector HitNormal;
    bool bHitGround = TraceForGround(FootSocketLocation, HitLocation, HitNormal);
    
    if (bHitGround)
    {
        // Calculate foot offset
        float FootOffset = CalculateFootOffset(FootSocketLocation, HitLocation);
        FootOffset = FMath::Clamp(FootOffset, -IKSettings.MaxFootOffset, IKSettings.MaxFootOffset);
        
        // Calculate foot rotation based on ground normal
        FRotator CurrentFootRotation = OwnerMesh->GetSocketRotation(FootBoneName);
        FRotator TargetFootRotation = CalculateFootRotation(HitNormal, CurrentFootRotation);
        
        // Set IK data
        IKData.FootLocation = HitLocation;
        IKData.FootRotation = TargetFootRotation;
        IKData.FootOffset = FootOffset;
        IKData.IKAlpha = 1.0f;
        IKData.bIsGrounded = true;
    }
    else
    {
        // No ground hit - disable IK for this foot
        IKData.IKAlpha = 0.0f;
        IKData.bIsGrounded = false;
    }
    
    return IKData;
}

bool UAnim_IKFootPlacement::TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Setup trace parameters
    FVector TraceStart = StartLocation + FVector(0, 0, 50.0f); // Start slightly above
    FVector TraceEnd = StartLocation - FVector(0, 0, IKSettings.TraceDistance);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    QueryParams.AddIgnoredActor(GetOwner());
    
    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        IKSettings.TraceChannel,
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

float UAnim_IKFootPlacement::CalculateFootOffset(const FVector& FootLocation, const FVector& HitLocation)
{
    float Offset = HitLocation.Z - FootLocation.Z + IKSettings.FootHeight;
    return FMath::Clamp(Offset, -IKSettings.MaxFootOffset, IKSettings.MaxFootOffset);
}

FRotator UAnim_IKFootPlacement::CalculateFootRotation(const FVector& HitNormal, const FRotator& CurrentRotation)
{
    // Calculate the rotation needed to align foot with ground normal
    FVector ForwardVector = CurrentRotation.Vector();
    FVector RightVector = FVector::CrossProduct(HitNormal, ForwardVector).GetSafeNormal();
    ForwardVector = FVector::CrossProduct(RightVector, HitNormal).GetSafeNormal();
    
    FRotator TargetRotation = UKismetMathLibrary::MakeRotationFromAxes(ForwardVector, RightVector, HitNormal);
    
    // Limit rotation to prevent extreme angles
    FRotator DeltaRotation = TargetRotation - CurrentRotation;
    DeltaRotation.Pitch = FMath::Clamp(DeltaRotation.Pitch, -45.0f, 45.0f);
    DeltaRotation.Roll = FMath::Clamp(DeltaRotation.Roll, -45.0f, 45.0f);
    
    return CurrentRotation + DeltaRotation;
}

void UAnim_IKFootPlacement::UpdatePelvisOffset()
{
    // Calculate pelvis offset based on foot offsets
    float LeftOffset = LeftFootIK.FootOffset;
    float RightOffset = RightFootIK.FootOffset;
    
    // Use the higher foot as reference (lower offset value)
    float TargetPelvisOffset = FMath::Min(LeftOffset, RightOffset);
    
    // Smooth interpolation
    PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvisOffset, GetWorld()->GetDeltaSeconds(), IKSettings.InterpSpeed);
}

void UAnim_IKFootPlacement::InterpolateIKData(FAnim_FootIKData& CurrentData, const FAnim_FootIKData& TargetData, float DeltaTime)
{
    float InterpSpeed = IKSettings.InterpSpeed;
    
    CurrentData.FootOffset = FMath::FInterpTo(CurrentData.FootOffset, TargetData.FootOffset, DeltaTime, InterpSpeed);
    CurrentData.IKAlpha = FMath::FInterpTo(CurrentData.IKAlpha, TargetData.IKAlpha, DeltaTime, InterpSpeed * 2.0f);
    CurrentData.FootLocation = FMath::VInterpTo(CurrentData.FootLocation, TargetData.FootLocation, DeltaTime, InterpSpeed);
    CurrentData.FootRotation = FMath::RInterpTo(CurrentData.FootRotation, TargetData.FootRotation, DeltaTime, InterpSpeed);
    CurrentData.bIsGrounded = TargetData.bIsGrounded;
}

FVector UAnim_IKFootPlacement::GetSocketLocation(const FName& SocketName)
{
    if (OwnerMesh && OwnerMesh->DoesSocketExist(SocketName))
    {
        return OwnerMesh->GetSocketLocation(SocketName);
    }
    
    return GetOwner()->GetActorLocation();
}

void UAnim_IKFootPlacement::DrawDebugInfo()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw foot trace lines
    FVector LeftFootLoc = GetSocketLocation(LeftFootBoneName);
    FVector RightFootLoc = GetSocketLocation(RightFootBoneName);
    
    DrawDebugLine(GetWorld(), LeftFootLoc + FVector(0, 0, 50), LeftFootLoc - FVector(0, 0, IKSettings.TraceDistance), 
                  LeftFootIK.bIsGrounded ? FColor::Green : FColor::Red, false, 0.1f, 0, 2.0f);
    
    DrawDebugLine(GetWorld(), RightFootLoc + FVector(0, 0, 50), RightFootLoc - FVector(0, 0, IKSettings.TraceDistance), 
                  RightFootIK.bIsGrounded ? FColor::Green : FColor::Red, false, 0.1f, 0, 2.0f);
    
    // Draw foot positions
    if (LeftFootIK.bIsGrounded)
    {
        DrawDebugSphere(GetWorld(), LeftFootIK.FootLocation, 5.0f, 12, FColor::Blue, false, 0.1f);
    }
    
    if (RightFootIK.bIsGrounded)
    {
        DrawDebugSphere(GetWorld(), RightFootIK.FootLocation, 5.0f, 12, FColor::Blue, false, 0.1f);
    }
}

void UAnim_IKFootPlacement::SetIKSettings(const FAnim_IKSettings& NewSettings)
{
    IKSettings = NewSettings;
}

void UAnim_IKFootPlacement::EnableIK(bool bEnable)
{
    bIKEnabled = bEnable;
    
    if (!bIKEnabled)
    {
        ResetIK();
    }
}

void UAnim_IKFootPlacement::ResetIK()
{
    LeftFootIK = FAnim_FootIKData();
    RightFootIK = FAnim_FootIKData();
    PelvisOffset = 0.0f;
}