#include "Light_UnderwaterCausticsSystem.h"
#include "Engine/Engine.h"
#include "Components/RectLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"

ULight_UnderwaterCausticsSystem::ULight_UnderwaterCausticsSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.033f; // 30 FPS for caustics animation
}

void ULight_UnderwaterCausticsSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeUnderwaterZones();
}

void ULight_UnderwaterCausticsSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (UnderwaterZones.Num() > 0)
    {
        UpdateCausticsAnimation(DeltaTime);
        OptimizeCausticsPerformance();
    }
}

void ULight_UnderwaterCausticsSystem::InitializeUnderwaterZones()
{
    // Create default underwater zone if none exist
    if (UnderwaterZones.Num() == 0)
    {
        FLight_UnderwaterZone DefaultZone;
        DefaultZone.ZoneCenter = FVector(2000.0f, 2000.0f, -200.0f);
        DefaultZone.ZoneExtent = FVector(3000.0f, 3000.0f, 800.0f);
        DefaultZone.WaterSurfaceHeight = 0.0f;
        DefaultZone.CausticsConfig.CausticsIntensity = 2.5f;
        DefaultZone.CausticsConfig.WaveSpeed = 0.8f;
        DefaultZone.CausticsConfig.WaveAmplitude = 0.4f;
        UnderwaterZones.Add(DefaultZone);
    }
    
    GenerateCausticsPattern();
}

void ULight_UnderwaterCausticsSystem::UpdateCausticsAnimation(float DeltaTime)
{
    CurrentWaveTime += DeltaTime * CausticsAnimationSpeed;
    
    for (int32 i = 0; i < CausticsLights.Num(); ++i)
    {
        if (CausticsLights[i])
        {
            // Animate caustics intensity with wave patterns
            float WavePattern1 = FMath::Sin(CurrentWaveTime * 2.0f + i * 0.5f) * 0.3f;
            float WavePattern2 = FMath::Cos(CurrentWaveTime * 1.5f + i * 0.8f) * 0.2f;
            float AnimatedIntensity = 2.0f + WavePattern1 + WavePattern2;
            
            CausticsLights[i]->SetIntensity(AnimatedIntensity * 1000.0f);
            
            // Animate light position for moving caustics
            FVector BasePosition = CausticsLights[i]->GetComponentLocation();
            float OffsetX = FMath::Sin(CurrentWaveTime * 0.7f + i) * 50.0f;
            float OffsetY = FMath::Cos(CurrentWaveTime * 0.9f + i) * 30.0f;
            CausticsLights[i]->SetWorldLocation(BasePosition + FVector(OffsetX, OffsetY, 0.0f));
        }
    }
    
    UpdateWaterSurfaceNormals();
}

void ULight_UnderwaterCausticsSystem::SetUnderwaterZone(const FLight_UnderwaterZone& NewZone)
{
    UnderwaterZones.Empty();
    UnderwaterZones.Add(NewZone);
    GenerateCausticsPattern();
}

bool ULight_UnderwaterCausticsSystem::IsLocationUnderwater(const FVector& Location) const
{
    for (const FLight_UnderwaterZone& Zone : UnderwaterZones)
    {
        if (!Zone.bIsActive) continue;
        
        FVector RelativePos = Location - Zone.ZoneCenter;
        if (FMath::Abs(RelativePos.X) <= Zone.ZoneExtent.X &&
            FMath::Abs(RelativePos.Y) <= Zone.ZoneExtent.Y &&
            Location.Z < Zone.WaterSurfaceHeight &&
            Location.Z > (Zone.ZoneCenter.Z - Zone.ZoneExtent.Z))
        {
            return true;
        }
    }
    return false;
}

float ULight_UnderwaterCausticsSystem::GetWaterDepthAtLocation(const FVector& Location) const
{
    for (const FLight_UnderwaterZone& Zone : UnderwaterZones)
    {
        if (IsLocationUnderwater(Location))
        {
            return FMath::Max(0.0f, Zone.WaterSurfaceHeight - Location.Z);
        }
    }
    return 0.0f;
}

void ULight_UnderwaterCausticsSystem::CreateCausticsLightPattern(const FVector& SurfaceLocation, float Intensity)
{
    if (CausticsLights.Num() >= MaxActiveCausticsLights)
    {
        return; // Performance limit reached
    }
    
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    // Create rect light for caustics projection
    URectLightComponent* CausticsLight = NewObject<URectLightComponent>(Owner);
    if (CausticsLight)
    {
        CausticsLight->AttachToComponent(Owner->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
        
        CausticsLight->SetWorldLocation(SurfaceLocation + FVector(0, 0, 200));
        CausticsLight->SetWorldRotation(FRotator(-90, 0, 0));
        CausticsLight->SetIntensity(Intensity * 1000.0f);
        CausticsLight->SetLightColor(FLinearColor(0.4f, 0.8f, 1.0f, 1.0f));
        CausticsLight->SetSourceWidth(400.0f);
        CausticsLight->SetSourceHeight(400.0f);
        CausticsLight->SetBarnDoorAngle(30.0f);
        
        CausticsLights.Add(CausticsLight);
        CausticsLight->RegisterComponent();
    }
}

void ULight_UnderwaterCausticsSystem::UpdateUnderwaterAmbientLighting(float DepthFactor)
{
    // Reduce ambient lighting based on water depth
    float AmbientReduction = FMath::Clamp(DepthFactor * LightAbsorptionRate, 0.0f, 0.9f);
    
    // Apply blue-green tint to simulate water filtering
    FLinearColor UnderwaterTint = FLinearColor::LerpUsingHSV(
        FLinearColor::White,
        WaterAbsorptionColor,
        AmbientReduction
    );
    
    // This would typically be applied to a post-process volume or global lighting
    UE_LOG(LogTemp, Log, TEXT("Underwater ambient lighting: Depth=%f, Tint=(%f,%f,%f)"), 
           DepthFactor, UnderwaterTint.R, UnderwaterTint.G, UnderwaterTint.B);
}

void ULight_UnderwaterCausticsSystem::GenerateCausticsPattern()
{
    // Clear existing caustics lights
    for (URectLightComponent* Light : CausticsLights)
    {
        if (Light)
        {
            Light->DestroyComponent();
        }
    }
    CausticsLights.Empty();
    
    // Generate caustics pattern for each underwater zone
    for (const FLight_UnderwaterZone& Zone : UnderwaterZones)
    {
        if (!Zone.bIsActive) continue;
        
        int32 CausticsCount = FMath::Min(6, MaxActiveCausticsLights - CausticsLights.Num());
        for (int32 i = 0; i < CausticsCount; ++i)
        {
            FVector CausticsPos = Zone.ZoneCenter + FVector(
                FMath::RandRange(-Zone.ZoneExtent.X * 0.8f, Zone.ZoneExtent.X * 0.8f),
                FMath::RandRange(-Zone.ZoneExtent.Y * 0.8f, Zone.ZoneExtent.Y * 0.8f),
                Zone.WaterSurfaceHeight
            );
            
            CreateCausticsLightPattern(CausticsPos, Zone.CausticsConfig.CausticsIntensity);
        }
    }
}

void ULight_UnderwaterCausticsSystem::UpdateCausticsIntensity(float NewIntensity)
{
    for (URectLightComponent* Light : CausticsLights)
    {
        if (Light)
        {
            Light->SetIntensity(NewIntensity * 1000.0f);
        }
    }
}

void ULight_UnderwaterCausticsSystem::SetCausticsWaveParameters(float Speed, float Amplitude)
{
    CausticsAnimationSpeed = Speed;
    
    for (FLight_UnderwaterZone& Zone : UnderwaterZones)
    {
        Zone.CausticsConfig.WaveSpeed = Speed;
        Zone.CausticsConfig.WaveAmplitude = Amplitude;
    }
}

void ULight_UnderwaterCausticsSystem::CalculateWaterSurfaceRefraction(const FVector& LightDirection, FVector& RefractedDirection)
{
    // Snell's law implementation for water refraction
    float RefractiveIndex = 1.33f; // Water refractive index
    FVector Normal = FVector::UpVector; // Simplified water surface normal
    
    float CosI = FVector::DotProduct(-LightDirection, Normal);
    float SinT2 = (1.0f / (RefractiveIndex * RefractiveIndex)) * (1.0f - CosI * CosI);
    
    if (SinT2 <= 1.0f)
    {
        float CosT = FMath::Sqrt(1.0f - SinT2);
        RefractedDirection = (LightDirection / RefractiveIndex) + 
                           ((CosI / RefractiveIndex) - CosT) * Normal;
        RefractedDirection.Normalize();
    }
    else
    {
        // Total internal reflection
        RefractedDirection = LightDirection - 2.0f * FVector::DotProduct(LightDirection, Normal) * Normal;
    }
}

void ULight_UnderwaterCausticsSystem::UpdateWaterSurfaceNormals()
{
    // Update water surface normals for realistic caustics patterns
    for (int32 i = 0; i < UnderwaterZones.Num(); ++i)
    {
        FLight_UnderwaterZone& Zone = UnderwaterZones[i];
        if (!Zone.bIsActive) continue;
        
        // Calculate wave-based surface normal variations
        float WaveHeight = CalculateWaterSurfaceHeight(Zone.ZoneCenter, CurrentWaveTime);
        Zone.WaterSurfaceHeight = Zone.ZoneCenter.Z + WaveHeight;
    }
}

void ULight_UnderwaterCausticsSystem::ApplyWaterColorGrading(float Depth)
{
    // Apply depth-based color grading for underwater effect
    float DepthFactor = FMath::Clamp(Depth / 1000.0f, 0.0f, 1.0f);
    
    // Simulate light absorption in water (red light absorbed first)
    FLinearColor DepthColor = FLinearColor(
        1.0f - DepthFactor * 0.8f,  // Red absorption
        1.0f - DepthFactor * 0.4f,  // Green absorption  
        1.0f - DepthFactor * 0.2f,  // Blue absorption (least)
        1.0f
    );
    
    UE_LOG(LogTemp, Log, TEXT("Water color grading at depth %f: (%f,%f,%f)"), 
           Depth, DepthColor.R, DepthColor.G, DepthColor.B);
}

void ULight_UnderwaterCausticsSystem::SetCausticsLOD(int32 LODLevel)
{
    // Adjust caustics quality based on LOD level
    switch (LODLevel)
    {
        case 0: // High quality
            MaxActiveCausticsLights = 12;
            PrimaryComponentTick.TickInterval = 0.033f;
            break;
        case 1: // Medium quality
            MaxActiveCausticsLights = 8;
            PrimaryComponentTick.TickInterval = 0.05f;
            break;
        case 2: // Low quality
            MaxActiveCausticsLights = 4;
            PrimaryComponentTick.TickInterval = 0.1f;
            break;
        default:
            MaxActiveCausticsLights = 6;
            PrimaryComponentTick.TickInterval = 0.033f;
            break;
    }
}

void ULight_UnderwaterCausticsSystem::UpdateCausticsVisibility(const FVector& ViewerLocation)
{
    // Enable/disable caustics based on viewer distance
    for (URectLightComponent* Light : CausticsLights)
    {
        if (Light)
        {
            float Distance = FVector::Dist(ViewerLocation, Light->GetComponentLocation());
            bool bShouldBeVisible = Distance <= MaxCausticsDistance;
            Light->SetVisibility(bShouldBeVisible);
        }
    }
}

void ULight_UnderwaterCausticsSystem::CalculateCausticsIntensity(const FVector& Position, float& OutIntensity)
{
    OutIntensity = 1.0f;
    
    for (const FLight_UnderwaterZone& Zone : UnderwaterZones)
    {
        if (!Zone.bIsActive) continue;
        
        float Depth = GetWaterDepthAtLocation(Position);
        if (Depth > 0.0f)
        {
            // Intensity decreases with depth
            float DepthFactor = FMath::Exp(-Depth * LightAbsorptionRate * 0.001f);
            
            // Add wave-based caustics pattern
            float WavePattern = CalculateWaterSurfaceHeight(Position, CurrentWaveTime);
            float CausticsMultiplier = 1.0f + WavePattern * Zone.CausticsConfig.WaveAmplitude;
            
            OutIntensity = DepthFactor * CausticsMultiplier * Zone.CausticsConfig.CausticsIntensity;
            break;
        }
    }
}

void ULight_UnderwaterCausticsSystem::UpdateCausticsLightPositions()
{
    // Update light positions based on water surface waves
    for (int32 i = 0; i < CausticsLights.Num(); ++i)
    {
        if (CausticsLights[i])
        {
            FVector CurrentPos = CausticsLights[i]->GetComponentLocation();
            float WaveOffset = CalculateWaterSurfaceHeight(CurrentPos, CurrentWaveTime);
            CausticsLights[i]->SetWorldLocation(FVector(CurrentPos.X, CurrentPos.Y, CurrentPos.Z + WaveOffset));
        }
    }
}

void ULight_UnderwaterCausticsSystem::OptimizeCausticsPerformance()
{
    // Remove lights that are too far from any underwater zone
    for (int32 i = CausticsLights.Num() - 1; i >= 0; --i)
    {
        if (!CausticsLights[i])
        {
            CausticsLights.RemoveAt(i);
            continue;
        }
        
        FVector LightPos = CausticsLights[i]->GetComponentLocation();
        bool bNearUnderwaterZone = false;
        
        for (const FLight_UnderwaterZone& Zone : UnderwaterZones)
        {
            if (FVector::Dist(LightPos, Zone.ZoneCenter) <= Zone.ZoneExtent.Size())
            {
                bNearUnderwaterZone = true;
                break;
            }
        }
        
        if (!bNearUnderwaterZone)
        {
            CausticsLights[i]->DestroyComponent();
            CausticsLights.RemoveAt(i);
        }
    }
}

float ULight_UnderwaterCausticsSystem::CalculateWaterSurfaceHeight(const FVector& Position, float Time)
{
    // Simple wave simulation for water surface
    float Wave1 = FMath::Sin(Position.X * 0.01f + Time * 2.0f) * 20.0f;
    float Wave2 = FMath::Cos(Position.Y * 0.008f + Time * 1.5f) * 15.0f;
    float Wave3 = FMath::Sin((Position.X + Position.Y) * 0.005f + Time * 3.0f) * 10.0f;
    
    return (Wave1 + Wave2 + Wave3) * 0.1f;
}

FVector ULight_UnderwaterCausticsSystem::CalculateWaterSurfaceNormal(const FVector& Position, float Time)
{
    // Calculate surface normal from wave height gradients
    float Epsilon = 1.0f;
    float HeightX1 = CalculateWaterSurfaceHeight(Position + FVector(Epsilon, 0, 0), Time);
    float HeightX2 = CalculateWaterSurfaceHeight(Position - FVector(Epsilon, 0, 0), Time);
    float HeightY1 = CalculateWaterSurfaceHeight(Position + FVector(0, Epsilon, 0), Time);
    float HeightY2 = CalculateWaterSurfaceHeight(Position - FVector(0, Epsilon, 0), Time);
    
    FVector Normal = FVector(
        -(HeightX1 - HeightX2) / (2.0f * Epsilon),
        -(HeightY1 - HeightY2) / (2.0f * Epsilon),
        1.0f
    );
    
    return Normal.GetSafeNormal();
}

void ULight_UnderwaterCausticsSystem::TraceLightRayThroughWater(const FVector& StartPos, const FVector& Direction, FVector& OutHitPos, float& OutIntensity)
{
    // Simple light ray tracing through water medium
    FVector CurrentPos = StartPos;
    FVector RayDirection = Direction;
    OutIntensity = 1.0f;
    
    float StepSize = 50.0f;
    int32 MaxSteps = 20;
    
    for (int32 Step = 0; Step < MaxSteps; ++Step)
    {
        CurrentPos += RayDirection * StepSize;
        
        if (IsLocationUnderwater(CurrentPos))
        {
            float Depth = GetWaterDepthAtLocation(CurrentPos);
            OutIntensity *= FMath::Exp(-LightAbsorptionRate * StepSize * 0.01f);
            
            // Apply refraction at water surface
            if (Step == 0)
            {
                FVector RefractedDir;
                CalculateWaterSurfaceRefraction(RayDirection, RefractedDir);
                RayDirection = RefractedDir;
            }
        }
        else
        {
            break;
        }
    }
    
    OutHitPos = CurrentPos;
}